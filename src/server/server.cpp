// server
#include <conn_handler.hpp>
#include <server.hpp>

// unix (hopefully)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

// fmt
#include <fmt/core.h>

// std
#include <cstdlib>
#include <list>

// common
#include <abort.hpp>
#include <log.hpp>

// share
#include <share.hpp>

// threading
#include <threading.hpp>

#define BACKLOG (MAX_CONNS)

namespace server {

server_t::server_t(uint16_t port)
    : m_port(port)
{
}

void server_t::operator()()
{
#ifdef __APPLE__
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    int saved_flags = fcntl(m_socket_fd, F_GETFL);
    AbortIfV(saved_flags == -1,
        "updating the socket file descriptor "
        "failed, reason {}",
        strerror(errno));
    int is_non_blocking = fcntl(
        m_socket_fd, F_SETFL, saved_flags & O_NONBLOCK);
    AbortIfV(is_non_blocking == -1,
        "attempt to modify the connection file "
        "descriptor "
        "failed, reason {}",
        strerror(errno));
#else
    m_socket_fd
        = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
#endif

    if (m_socket_fd == -1) {
        log.error("could not create socket, reason: {}",
            strerror(errno));

        return;
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(m_port);

    if (bind(m_socket_fd, (sockaddr*)&server_addr,
            sizeof(server_addr))
        == -1) {
        log.error("could not bind socket, reason: {}",
            strerror(errno));

        // closing is handled in dtor();

        return;
    }

    // NOTE: the backlog parameter limits the number
    // of outstanding connections on the socket's listen
    // queue.
    // TODO: check if the backlog can fail us
    if (listen(m_socket_fd, BACKLOG) == -1) {
        log.error("could not listen on socket, reason: {}",
            strerror(errno));

        // closing is handled in dtor();

        return;
    }

    log.info("server listening on port {}", m_port);

    requests_loop();
}

void server_t::dtor()
{
    if (m_current_conn_fd != -1) {
        threading::mutex_guard guard {
            share::e_connections_mutex
        };

        if (shutdown(m_current_conn_fd, SHUT_WR) == -1) {
            log.error("connection shutdown failed, "
                      "reason: {}",
                strerror(errno));
        }

        if (close(m_current_conn_fd) == -1) {
            log.error(
                "closing connection failed, reason: {}",
                strerror(errno));
        }

        share::e_connections.erase(m_current_conn_fd);
    }

    for (auto& thread : share::e_threads) {
        thread.cancel();

        thread.join();
    }

    share::e_updater_thread.cancel();

    if (close(m_socket_fd) == -1) {
        log.error("closing socket failed, reason: {}",
            strerror(errno));
    }

    log.info("stopping server");

    log.flush();
}

void server_t::requests_loop()
{
    size_t num_of_conns { 0 };

    for (;;) {
        sockaddr_in addr {};

        socklen_t addr_len { sizeof(sockaddr_in) };

        pollfd poll_fd = { m_socket_fd, POLLIN, 0 };

        if (poll(&poll_fd, 1, -1) == -1) {
            if (errno == EINTR) {
                // TODO: add info about interrupt
                log.warn("poll interrupted");

                continue;
            }

            AbortV("poll of socket failed, reason: {}",
                strerror(errno));
        }

        if (!(poll_fd.revents & POLLIN)) {
            // TODO: investigate when this happens if at all
            log.warn("unexpected socket event");

            continue;
        }

        m_current_conn_fd = accept(
            m_socket_fd, (sockaddr*)&addr, &addr_len);

        if (m_current_conn_fd == -1) {
            log.warn("accepting incoming connection "
                     "failed, reason: {}",
                strerror(errno));

            continue;
        }

        pollfd check_poll_fd
            = { m_current_conn_fd, POLLOUT, 0 };

        if (poll(&check_poll_fd, 1, 60000) == -1) {
            log.error(
                "poll of connection failed, reason: {}",
                strerror(errno));

            if (close(m_current_conn_fd) == -1) {
                AbortV("closing connection failed, "
                       "reason: {}",
                    strerror(errno));
            }

            m_current_conn_fd = -1;

            continue;
        }

        if (check_poll_fd.revents & POLLHUP) {
            log.warn("connection hung up");

            if (close(m_current_conn_fd) == -1) {
                AbortV("closing connection failed, "
                       "reason: {}",
                    strerror(errno));
            }

            m_current_conn_fd = -1;

            continue;
        }

        if (!(check_poll_fd.revents & POLLOUT)) {
            log.warn("establishing connection timedout");

            if (close(m_current_conn_fd) == -1) {
                AbortV("closing connection failed, "
                       "reason: {}",
                    strerror(errno));
            }

            m_current_conn_fd = -1;

            continue;
        }

        uint16_t check = htons(1);

        if (num_of_conns >= MAX_CONNS) {
            check = htons(2);
        }

        ssize_t check_size = write(
            m_current_conn_fd, &check, sizeof(check));

        if (check_size == -1) {
            log.warn("reading from connection failed, "
                     "reason: {}",
                strerror(errno));

            if (close(m_current_conn_fd) == -1) {
                AbortV("closing connection failed, "
                       "reason: {}",
                    strerror(errno));
            }

            m_current_conn_fd = -1;

            continue;
        }

        if (check_size != sizeof(check)) {
            log.warn("unexpected message size ({} bytes)",
                check_size);

            if (close(m_current_conn_fd) == -1) {
                AbortV("closing connection failed, "
                       "reason: {}",
                    strerror(errno));
            }

            m_current_conn_fd = -1;

            continue;
        }

        if (num_of_conns >= MAX_CONNS) {
            if (close(m_current_conn_fd) == -1) {
                AbortV(
                    "closing connection failed, reason: {}",
                    strerror(errno));
            }

            m_current_conn_fd = -1;

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
        int saved_flags = fcntl(m_current_conn_fd, F_GETFL);
        AbortIfV(saved_flags == -1,
            "updating the connection file descriptor "
            "failed, reason {}",
            strerror(errno));
        int is_non_blocking = fcntl(m_current_conn_fd,
            F_SETFL, saved_flags & ~O_NONBLOCK);
        AbortIfV(is_non_blocking == -1,
            "attempt to modify the connection file "
            "descriptor "
            "failed, reason {}",
            strerror(errno));

        // NOTE: e_connection can be in the process
        // of being read from by the updater thread. So we
        // should lock
        {
            threading::mutex_guard guard {
                share::e_connections_mutex
            };

            share::e_connections.insert(m_current_conn_fd);
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
            threading::mutex_guard guard {
                share::e_threads_mutex
            };

            threading::pthread::test_cancel();

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

            share::e_threads.emplace_back(
                conn_handler_t { m_current_conn_fd, addr });

            m_current_conn_fd = -1;

            // trim any finished threads
            share::e_threads.erase(
                std::remove_if(share::e_threads.begin(),
                    share::e_threads.end(),
                    [](auto& thread) {
                        return !thread.is_alive();
                    }),
                share::e_threads.end());

            num_of_conns = share::e_threads.size();
        }

        log.flush();
    }
}

logging::log server_t::log {};

void server_t::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);
}

} // namespace server
