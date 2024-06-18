// server
#include "conn_handler.hpp"
#include "share.hpp"
#include "timing.hpp"

// unix
#include <arpa/inet.h>
#include <fmt/format.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

// std
#include <variant>

// common
#include "../common/abort.hpp"
#include "../common/overload.hpp"
#include "../common/threading.hpp"

// bench
#include "../bench/bench.hpp"

// spdlog
#include <spdlog/spdlog.h>

#define MINUTE (60000)

namespace server {

ConnHandler::ConnHandler(IPv4SocketRef sock, std::string username)
    : m_sock(sock), m_channel(m_sock), m_username(std::move(username))
{
}

void ConnHandler::setup_readable_net_info()
{
    auto addr = m_sock.get_sockaddr_in();

    char ipv4[INET_ADDRSTRLEN + 1];

    bzero(&ipv4, INET_ADDRSTRLEN + 1);

    if (inet_ntop(AF_INET, &addr.sin_addr, ipv4, INET_ADDRSTRLEN) == nullptr) {
        ABORTV("inet_ntop(...) failed, reason: {}", strerror(errno));
    }

    m_ipv4 = std::string { ipv4 };

    m_port = std::to_string(ntohs(addr.sin_port));
}

void ConnHandler::operator()()
{
    setup_readable_net_info();

    spdlog::info(
        "received a connection from {}:{} ({})",
        m_ipv4,
        m_port,
        m_username
    );

    pthread_cleanup_push(
        [](void* untyped_self) {
            auto* self = static_cast<ConnHandler*>(untyped_self);

            {
                threading::mutex_guard guard { share::connections_mutex };

                share::connections.erase(self->m_sock.native_handle());
            }

            {
                threading::mutex_guard guard { share::users_mutex };

                share::users.erase(self->m_username);
            }
        },
        this
    );

    {
        BENCH("sending the full list");

        if (!send_full_list()) {
            spdlog::info(
                "[{}:{} ({})] failed to send full list",
                m_ipv4,
                m_port,
                m_username
            );

            return;
        }
    }

    for (;;) {
        auto [res, status]
            = m_channel.read(static_cast<int>(MINUTE * share::time_out));

        if (status != ChannelErrorCode::OK) {
            spdlog::info(
                "[{}:{} ({})] reading failed, reason: {}",
                m_ipv4,
                m_port,
                m_username,
                status.what()
            );

            // NOTE: even when failing with END_OF_FILE, it
            // is marked as a failure in reading. However,
            // it is implicitly assumed that the user
            // disconnected.

            if (status == ChannelErrorCode::END_OF_FILE) {
                spdlog::info(
                    "[{}:{} ({})] assuming user disconnected",
                    m_ipv4,
                    m_port,
                    m_username,
                    status.what()
                );
            }

            break;
        }

        spdlog::debug(
            "[{}:{} ({})] payload size {} bytes",
            m_ipv4,
            m_port,
            m_username,
            res.size()
        );

        {
            BENCH("handling payload");

            handle_payload(res);
        }
    }

    create_client_timer(m_username);

    spdlog::info(
        "[{}:{} ({})] closing connection handler",
        m_ipv4,
        m_port,
        m_username
    );

    pthread_cleanup_pop(1);
}

bool ConnHandler::send_full_list()
{
    TaggedDrawVector tagged_draw_vector {};

    ChannelError status {};

    {
        threading::unique_mutex_guard guard { share::update_mutex };

        tagged_draw_vector = share::tagged_draw_vector;

        status = m_channel.write(serialize<Payload>(tagged_draw_vector));
    }

    if (status != ChannelErrorCode::OK) {
        spdlog::info(
            "[{}:{} ({})] writing failed, reason: {}",
            m_ipv4,
            m_port,
            m_username,
            status.what()
        );

        return false;
    }

    return true;
}

void ConnHandler::handle_payload(const ByteString& bytes)
{
    auto [payload, status] = deserialize<Payload>(bytes);

    if (status != DeserializeErrorCode::OK) {
        spdlog::warn(
            "[{}:{} ({})] deserialization failed, reason {}",
            m_ipv4,
            m_port,
            m_username,
            status.what()
        );
        return;
    }

    if (!std::holds_alternative<TaggedAction>(payload)) {
        spdlog::warn(
            "[{}:{} ({})] unexpected payload type {}",
            m_ipv4,
            m_port,
            m_username,
            var_type(payload).name()
        );

        return;
    }

    TaggedAction tagged_action = std::get<TaggedAction>(payload);

    // NOTE: if we block on the push queue we might actually
    // miss data or overflow data that is sent to us in
    // buffer the kernel has allocated for the socket.
    // That's what we are doing right now

    {
        threading::unique_mutex_guard guard { share::update_mutex };

        share::payload_queue.emplace(tagged_action);
    }

    share::update_cond.notify_one();
}

} // namespace server
