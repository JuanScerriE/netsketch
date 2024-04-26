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

// cstd
#include <cstdlib>

// std
#include <type_traits>

// common
#include "../common/abort.hpp"
#include "../common/channel.hpp"
#include "../common/log.hpp"
#include "../common/network.hpp"
#include "../common/serial.hpp"
#include "../common/threading.hpp"

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
        log.error(error.what());

        share::updater_thread.cancel();

        return;
    }

    log.info("server listening on port {}", m_port);

    request_loop();
}

void Server::request_loop()
{
    while (share::run) {
        PollResult poll_result {};

        try {
            poll_result = m_sock.poll({ POLLIN });
        } catch (std::runtime_error& error) {
            log.error(error.what());

            break;
        }

        if (poll_result.has_timed_out()) {
            continue;
        }

        IPv4Socket conn_sock {};

        try {
            conn_sock = m_sock.accept();
        } catch (std::runtime_error& error) {
            log.warn(error.what());

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
            threading::mutex_guard guard { share::timers_mutex };

            for (auto iter = share::timers.rbegin();
                 iter != share::timers.rend();
                 iter++) {
                if (iter->get()->user == *username) {
                    share::timers.erase(iter.base());

                    break;
                }
            }
        }

        IPv4SocketRef conn_sock_ref { conn_sock };

        // beyond this point it should be taken up
        // separately

        // NOTE: e_connection can be in the process
        // of being read from by the updater thread. So we
        // should lock
        {
            threading::mutex_guard guard { share::connections_mutex };

            share::connections[conn_sock.native_handle()]
                = std::move(conn_sock);
        }

        // Additionally, if we receive a SIGINT
        // here we'd have an open connection
        // but no conn_handler thread which will
        // close the connection upon exit.
        //
        // The above is wrong we only stop
        // at cancellation points if we receive a
        // cancel which good.

        {
            threading::mutex_guard guard { share::threads_mutex };

            // TODO: test whether this is necessary
            threading::thread::test_cancel();

            // So, what we'll do is we will test for
            // cancellation after acquiring the lock. This
            // ensures that if a cancellation happened
            // we just unwind and we never start the thread
            // and we never reset the m_current_conn_fd
            // essentially allowing us to close it in the
            // dtor of the server (this is fine since a
            // server can only handle one connection at a
            // time). We don't need to worry about it
            // being already pushed on the e_connections
            // list, since as far as I can tell it does
            // not yield any effects.
            //
            // TODO: test this out a bit more since there
            // is the possibility of more bugs
            //
            // Actually, all the above analysis is not
            // need the program will only stop at specified
            // cancellation points and because of this the
            // thread will be started. The only issue I have
            // with this approach is that it will go through
            // all of this and if it receives a cancel it
            // will open another thread which did not
            // receive a cancellation yet. Hence we have to
            // test for a cancellation before we actually
            // start the thread cancelled if cancelling if
            // properly done.

            share::threads.emplace_back(ConnHandler { conn_sock_ref });

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

        log.flush();
    }
}

std::optional<std::string> Server::is_valid_username(Channel channel)
{
    auto [res, read_status] = channel.read(60000);

    if (read_status != ChannelErrorCode::OK) {
        log.warn("reading failed, reason {}", read_status.what());

        return {};
    }

    Deserialize deserialize { res };

    auto [payload, deser_status] = deserialize.payload();

    if (deser_status != DeserializeErrorCode::OK) {
        log.warn("deserialization error occurred, {}", deser_status.what());

        return {};
    }

    if (!std::holds_alternative<Username>(payload)) {
        log.warn(
            "expected username payload, instead got {}",
            var_type(payload).name()
        );

        return {};
    }

    auto username = std::get<Username>(payload).username;

    if (m_users.count(username) > 0) {
        ByteVector req { Serialize {
            Decline {
                fmt::format("user with name {} already exists", username) } }
                             .bytes() };

        {
            auto status = channel.write(req);

            if (status != ChannelErrorCode::OK) {
                log.warn(
                    "responding failed reason {}",
                    var_type(payload).name()
                );

                return {};
            }
        }

        return {};
    }

    ByteVector req { Serialize { Accept {} }.bytes() };

    auto write_status = channel.write(req);

    if (write_status != ChannelErrorCode::OK) {
        log.warn("writing failed, reason {}", write_status.what());

        return {};
    }

    return std::make_optional(username);
}

// void Server::dtor()
// {
//     if (m_current_conn_fd != -1) {
//         threading::mutex_guard guard { share::e_connections_mutex };
//
//         if (shutdown(m_current_conn_fd, SHUT_WR) == -1) {
//             log.error(
//                 "connection shutdown failed, "
//                 "reason: {}",
//                 strerror(errno)
//             );
//         }
//
//         if (close(m_current_conn_fd) == -1) {
//             log.error("closing connection failed, reason: {}",
//             strerror(errno));
//         }
//
//         share::e_connections.erase(m_current_conn_fd);
//     }
//
//     for (auto& thread : share::e_threads) {
//         thread.cancel();
//
//         thread.join();
//     }
//
//     share::updater_thread.cancel();
//
//     if (close(m_socket_fd) == -1) {
//         log.error("closing socket failed, reason: {}", strerror(errno));
//     }
//
//     log.info("stopping server");
//
//     log.flush();
// }

logging::log Server::log {};

void Server::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);
}

} // namespace server
