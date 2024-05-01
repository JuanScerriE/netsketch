// server
#include "server.hpp"
#include "conn_handler.hpp"
#include "share.hpp"

// unix (hopefully)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <optional>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

// fmt
#include <fmt/core.h>

// common
#include "../common/channel.hpp"
#include "../common/network.hpp"
#include "../common/overload.hpp"
#include "../common/serial.hpp"
#include "../common/threading.hpp"

// bench
#include "../bench/bench.hpp"

// spdlog
#include <spdlog/spdlog.h>

#define BACKLOG (16)

namespace server {

Server::Server(uint16_t port)
    : m_port(port)
{
}

void Server::operator()()
{
    try {
        m_sock.open(SOCK_STREAM, 0);

        sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(m_port);

        m_sock.bind(&server_addr);

        // TODO: check if the backlog can fail us
        m_sock.listen(BACKLOG);
    } catch (std::runtime_error& error) {
        spdlog::error(error.what());

        share::updater_thread.cancel();

        return;
    }

    spdlog::info("server listening on port {}", m_port);

    request_loop();
}

void Server::request_loop()
{
    pthread_cleanup_push(
        [](void*) {
            {
                threading::mutex_guard guard { share::threads_mutex };

                for (auto& thread : share::threads) {
                    if (thread.is_initialized()) {
                        thread.join();
                    }
                }
            }

            {
                threading::mutex_guard guard { server::share::timers_mutex };

                server::share::timers.clear();
            }
        },
        this
    );

    for (;;) {
        PollResult poll_result {};

        try {
            poll_result = m_sock.poll({ POLLIN });
        } catch (std::runtime_error& error) {
            spdlog::error(error.what());

            break;
        }

        BENCH("handling incoming request");

        if (poll_result.has_timed_out()) {
            continue;
        }

        IPv4Socket conn_sock {};

        try {
            conn_sock = m_sock.accept();
        } catch (std::runtime_error& error) {
            spdlog::warn(error.what());

            continue;
        }

        // Make sure that the connection is
        // blocking just to be safe, you never
        // know someone might try to run
        // the code on System V or BSD, God knows.
        //
        // Due to the Versions section in
        // the man page accept(2):
        //    On Linux, the new socket returned by accept()
        //    does not inherit file status flags such as
        //    O_NONBLOCK and O_ASYNC from the listening
        //    socket.  This behavior differs from the
        //    canonical BSD sockets implementation. Portable
        //    programs should not rely on inheritance or
        //    noninheritance of file status flags and always
        //    explicitly set all required flags on the
        //    socket returned from accept().

        conn_sock.make_blocking();

        auto username = is_valid_username(Channel { conn_sock });

        if (!username.has_value()) {
            continue;
        }

        {
            threading::mutex_guard guard { share::users_mutex };

            share::users.insert(*username);
        }

        {
            threading::mutex_guard guard { share::timers_mutex };

            for (auto iter = share::timers.cbegin();
                 iter != share::timers.cend();
                 iter++) {
                if (iter->get()->user == *username) {
                    share::timers.erase(iter);

                    spdlog::info("{} reconnected", *username);

                    break;
                }
            }
        }

        IPv4SocketRef conn_sock_ref { conn_sock };

        // Handling the connection

        // NOTE: e_connection can be in the process
        // of being read from by the updater thread. So we
        // should lock

        {
            threading::mutex_guard guard { share::connections_mutex };

            share::connections[conn_sock.native_handle()]
                = std::move(conn_sock);
        }

        {
            threading::mutex_guard guard { share::threads_mutex };

            // we do not want to create a thread if cancellation has occurred
            // as that thread would not be cancelled.
            threading::thread::test_cancel();

            share::threads.emplace_back(ConnHandler { conn_sock_ref, *username }
            );

            // trim any finished threads
            share::threads.erase(
                std::remove_if(
                    share::threads.begin(),
                    share::threads.end(),
                    [](auto& thread) {
                        return !thread.is_alive();
                    }
                ),
                share::threads.end()
            );
        }
    }

    pthread_cleanup_pop(1);
}

std::optional<std::string> Server::is_valid_username(Channel channel)
{
    auto [res, read_status] = channel.read(60000);

    if (read_status != ChannelErrorCode::OK) {
        spdlog::warn("reading failed, reason {}", read_status.what());

        return {};
    }

    auto [payload, deser_status] = deserialize<Payload>(res);

    if (deser_status != DeserializeErrorCode::OK) {
        spdlog::warn("deserialization error occurred, {}", deser_status.what());

        return {};
    }

    if (!std::holds_alternative<Username>(payload)) {
        spdlog::warn(
            "expected username payload, instead got {}",
            var_type(payload).name()
        );

        return {};
    }

    auto username = std::get<Username>(payload).username;

    bool is_valid { true };

    {
        threading::mutex_guard guard { share::users_mutex };

        is_valid = share::users.count(username) > 0;
    }

    if (is_valid) {
        ByteString req { serialize<Payload>(Decline {
            fmt::format("user with name {} already exists", username) }) };

        {
            auto status = channel.write(req);

            if (status != ChannelErrorCode::OK) {
                spdlog::warn(
                    "responding failed reason {}",
                    var_type(payload).name()
                );

                return {};
            }
        }

        return {};
    }

    ByteString req { serialize<Payload>(Accept {}) };

    auto write_status = channel.write(req);

    if (write_status != ChannelErrorCode::OK) {
        spdlog::warn("writing failed, reason {}", write_status.what());

        return {};
    }

    return std::make_optional(username);
}

} // namespace server
