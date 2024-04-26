// std
#include <any>
#include <atomic>
#include <bits/chrono.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

// log
#include <log.hpp>

// arpa
#include <arpa/inet.h>

// network
#include "../common/serial.hpp"
#include "../common/types.hpp"
#include "abort.hpp"
#include "network.hpp"

#define MAGIC_BYTES (static_cast<short>(0x2003))

struct Header {
    std::uint16_t magic_bytes;
    std::size_t payload_size;

    [[nodiscard]] ByteVector serialize() const
    {
        FSerial fserial {};

        fserial.write(magic_bytes);
        fserial.write(payload_size);

        return fserial.bytes();
    }

    [[nodiscard]] static Header deserialize(ByteVector bytes)
    {
        BSerial bserial { std::move(bytes) };

        auto magic_bytes = bserial.read<decltype(Header::magic_bytes)>();
        auto payload_size = bserial.read<decltype(Header::payload_size)>();

        return { magic_bytes, payload_size };
    }
};

class Channel {
    using milliseconds = std::chrono::milliseconds;

   public:
    explicit Channel(IPv4Socket& conn_sock, milliseconds time_limit)
        : m_conn_sock { conn_sock }
        , m_is_time_limited { true }
        , m_time_limit { time_limit }
    {
        setup();
    }

    explicit Channel(IPv4Socket& conn_sock)
        : m_conn_sock(conn_sock), m_is_time_limited { false }, m_time_limit {}
    {
        setup();
    }

    void setup()
    {
        const struct sockaddr_in& conn_addr = m_conn_sock.get_sockaddr_in();

        char ipv4[INET_ADDRSTRLEN + 1];

        bzero(ipv4, sizeof(ipv4));

        if (inet_ntop(AF_INET, &conn_addr.sin_addr, ipv4, INET_ADDRSTRLEN)
            == nullptr) {
            ABORTV("inet_ntop(): {}", strerror(errno));
        }

        m_ipv4 = std::string { ipv4 };

        m_port = std::to_string(ntohs(conn_addr.sin_port));

        setup_logging(m_ipv4, m_port);

        m_reader = std::thread { [this] {
            reader();
        } };
        m_writer = std::thread { [this] {
            writer();
        } };
    }

    void reader()
    {
        using namespace std::chrono_literals;

        using namespace std::chrono;

        auto drop_point { time_point_cast<milliseconds>(system_clock::now()) };

        if (m_is_time_limited) {
            drop_point += m_time_limit;
        }

        while (m_run) {
            auto poll_result = m_conn_sock.poll({ POLLIN });

            if (poll_result.has_error()) {
                ABORTV(
                    "poll of socket failed, reason: {}",
                    poll_result.get_error()
                );
            }

            if (poll_result.has_timed_out()) {
                if (m_is_time_limited) {
                    auto current_time
                        = time_point_cast<milliseconds>(system_clock::now());

                    if (drop_point <= current_time) {
                        log.info("closing TCP connection");

                        break;
                    }
                }

                continue;
            }

            if (poll_result.is_hup()) {
                log.info("connection hung up");

                break;
            }

            if (m_is_time_limited) {
                drop_point = time_point_cast<milliseconds>(system_clock::now())
                             + m_time_limit;
            }

            ByteVector payload {};

            try {
                ReadResult result {};

                result = m_conn_sock.read(sizeof(Header));

                if (result.is_eof()) {
                    log.warn("closing tcp connection");

                    break;
                }

                auto header = Header::deserialize(result.get_bytes());

                if (header.magic_bytes != MAGIC_BYTES) {
                    log.warn(
                        "incorrect magic bytes, expected: "
                        "{}, got: {}",
                        MAGIC_BYTES,
                        header.magic_bytes
                    );

                    continue;
                }

                payload.reserve(header.payload_size);

                result = m_conn_sock.read(header.payload_size);

                if (result.is_eof()) {
                    log.warn("closing tcp connection");

                    break;
                }
            } catch (std::runtime_error& error) {
                log.warn(error.what());
            }

            handle(payload);

            log.flush();
        }

        shutdown();

        m_run = false;
    }

    virtual void handle(ByteVector bytes) = 0;

    void writer()
    {
        using namespace std::chrono_literals;

        while (m_run) {
            ByteVector payload {};

            {
                std::cv_status status { std::cv_status::no_timeout };

                std::unique_lock lock { m_mutex };

                while (m_writer_queue.empty()) {
                    status = m_cond_var.wait_for(lock, 128ms);

                    if (status == std::cv_status::timeout) {
                        break;
                    }
                }

                if (status == std::cv_status::timeout) {
                    continue;
                }

                payload = std::move(m_writer_queue.front());

                m_writer_queue.pop();
            }

            ByteVector header {
                Header { MAGIC_BYTES, payload.size() }.serialize()
            };

            ByteVector packet {};

            packet.reserve(header.size() + payload.size());

            for (auto& byte : header) {
                packet.push_back(byte);
            }

            for (auto& byte : payload) {
                packet.push_back(byte);
            }

            log.debug("packet size {}", packet.size());

            auto poll_result = m_conn_sock.poll({ POLLOUT });

            if (poll_result.has_error()) {
                ABORTV(
                    "poll of socket failed, reason: {}",
                    poll_result.get_error()
                );
            }

            if (poll_result.has_timed_out()) {
                continue;
            }

            if (poll_result.is_hup()) {
                log.info("connection hung up");

                break;
            }

            m_conn_sock.write(packet);

            log.flush();
        }

        shutdown();

        m_run = false;
    }

    virtual void send(std::any object) = 0;

    virtual void shutdown() = 0;

    virtual ~Channel()
    {
        m_run = false;

        m_reader.join();
        m_writer.join();
    };

    [[nodiscard]] bool is_running() const
    {
        return m_run;
    }

   protected:
    std::mutex m_mutex {};

    std::condition_variable m_cond_var {};

    std::queue<ByteVector> m_writer_queue {};

    // logging
    inline static logging::log log;

   private:
    static void setup_logging(std::string ipv4, std::string port)
    {
        using namespace logging;

        log.set_level(log::level::debug);

        log.set_prefix(fmt::format("[{}:{}]", ipv4, port));
    }

    std::atomic_bool m_run { true };

    std::thread m_reader {};
    std::thread m_writer {};

    IPv4Socket& m_conn_sock;

    bool m_is_time_limited;

    std::chrono::milliseconds m_time_limit;

    std::string m_ipv4;
    std::string m_port;
};
