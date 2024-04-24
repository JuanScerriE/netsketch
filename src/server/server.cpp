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

// common
#include <abort.hpp>
#include <log.hpp>

// share
#include <share.hpp>

// threading
#include <threading.hpp>

// network
#include <network.hpp>

#define BACKLOG (MAX_CONNS)

namespace server {

server_t::server_t(uint16_t port)
    : m_port(port)
{
}

void server_t::operator()()
{
    try {
        m_sock.open(SOCK_STREAM, 0);

        m_sock.make_non_blocking();

        sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(m_port);

        m_sock.bind(&server_addr);

        // TODO: check if the backlog can fail us
        m_sock.listen(BACKLOG);
    } catch (std::runtime_error& error) {
        log.error(error.what());

        return;
    }

    log.info("server listening on port {}", m_port);

    request_loop();
}

void server_t::request_loop()
{
    size_t num_of_conns { 0 };

    while (share::run) {
        auto result = m_sock.poll(POLLIN);

        if (result.has_error()) {
            // do not handle CTRL-C
            // if (errno == EINTR) {
            //     log.warn("poll interrupted");
            //
            //     continue;
            // }

            ABORTV(
                "poll of socket failed, reason: {}",
                result.get_error()
            );
        }

        if (result.has_timed_out()) {
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

        // beyond this point it should be taken up
        // separately

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
                conn_handler_t { m_current_conn_fd, addr }
            );

            m_current_conn_fd = -1;

            // trim any finished threads
            share::e_threads.erase(
                std::remove_if(
                    share::e_threads.begin(),
                    share::e_threads.end(),
                    [](auto& thread) {
                        return !thread.is_alive();
                    }
                ),
                share::e_threads.end()
            );

            num_of_conns = share::e_threads.size();
        }

        log.flush();
    }
}

void server_t::dtor()
{
    if (m_current_conn_fd != -1) {
        threading::mutex_guard guard {
            share::e_connections_mutex
        };

        if (shutdown(m_current_conn_fd, SHUT_WR) == -1) {
            log.error(
                "connection shutdown failed, "
                "reason: {}",
                strerror(errno)
            );
        }

        if (close(m_current_conn_fd) == -1) {
            log.error(
                "closing connection failed, reason: {}",
                strerror(errno)
            );
        }

        share::e_connections.erase(m_current_conn_fd);
    }

    for (auto& thread : share::e_threads) {
        thread.cancel();

        thread.join();
    }

    share::e_updater_thread.cancel();

    if (close(m_socket_fd) == -1) {
        log.error(
            "closing socket failed, reason: {}",
            strerror(errno)
        );
    }

    log.info("stopping server");

    log.flush();
}

logging::log server_t::log {};

void server_t::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);
}

} // namespace server
