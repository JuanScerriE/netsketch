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
#include <future>
#include <list>

// common
#include <abort.hpp>
#include <log.hpp>

// share
#include <share.hpp>

#define BACKLOG (16)

namespace server {

server_t::server_t(uint16_t port)
    : m_port(port) {
}

int server_t::operator()() {
#ifdef __APPLE__
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    int saved_flags = fcntl(m_socket_fd, F_GETFL);
    AbortIfV(
        saved_flags == -1,
        "updating the socket file descriptor "
        "failed, reason {}",
        strerror(errno)
    );
    int is_non_blocking = fcntl(
        m_socket_fd,
        F_SETFL,
        saved_flags & O_NONBLOCK
    );
    AbortIfV(
        is_non_blocking == -1,
        "attempt to modify the connection file "
        "descriptor "
        "failed, reason {}",
        strerror(errno)
    );
#else
    m_socket_fd =
        socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
#endif

    if (m_socket_fd == -1) {
        log::error(
            "could not create socket, reason: {}",
            strerror(errno)
        );

        return EXIT_FAILURE;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(m_port);

    if (bind(
            m_socket_fd,
            (sockaddr*)&server_addr,
            sizeof(server_addr)
        ) == -1) {
        log::error(
            "could not bind socket, reason: {}",
            strerror(errno)
        );

        close(m_socket_fd);

        return EXIT_FAILURE;
    }

    // NOTE: the backlog parameter limits the number
    // of outstanding connections on the socket's listen
    // queue.
    if (listen(m_socket_fd, BACKLOG) == -1) {
        log::error(
            "could not listen on socket, reason: {}",
            strerror(errno)
        );

        close(m_socket_fd);

        return EXIT_FAILURE;
    }

    log::info("server listening on port {}", m_port);

    requests_loop();

    if (close(m_socket_fd) == -1) {
        log::error(
            "closing socket failed, reason: {}",
            strerror(errno)
        );

        return EXIT_FAILURE;
    }

    log::info("socket closed");

    return EXIT_SUCCESS;
}

void server_t::requests_loop() {
    std::list<std::future<void>> futures{};

    for (;;) {
        sockaddr_in addr{};
        socklen_t addr_len{sizeof(sockaddr_in)};

        // we need to poll here (these get
        // optimised out)
        constexpr nfds_t nfds{2};
        constexpr size_t socket_idx{0};
        constexpr size_t event_idx{1};

        pollfd poll_fds[nfds] = {
            {m_socket_fd, POLLIN, 0},
            {share::e_stop_event->read_fd(), POLLIN, 0},
        };

        if (poll(poll_fds, nfds, -1) == -1) {
            if (errno == EINTR) {
                log::warn("poll interrupted");

                continue;
            }

            AbortV(
                "poll of socket or stop event failed, "
                "reason: {}",
                strerror(errno)
            );
        }

        // we should first check for the
        // stop event, if we ignore m_socket_fd
        // that is fine I think since
        // connections to the socket are first
        // placed on queue before they become
        // full connections

        if (poll_fds[event_idx].revents & POLLIN) {
            // break and stop everything
            break;
        }

        if (!(poll_fds[socket_idx].revents & POLLIN)) {
            // I think this should never be reached,
            // but I am not a 100% sure
            log::warn("unexpected socket event");

            continue;
        }

        int conn_fd = accept(
            m_socket_fd,
            (sockaddr*)&addr,
            &addr_len
        );

        if (conn_fd == -1) {
            log::warn(
                "accepting incoming connection "
                "failed, reason: {}",
                strerror(errno)
            );

            continue;
        }

        // Make sure that the connection is
        // blocking just to be safe, you never
        // know someone might try to run
        // your code on System V or BSD God knows.
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
        int saved_flags = fcntl(conn_fd, F_GETFL);
        AbortIfV(
            saved_flags == -1,
            "updating the connection file descriptor "
            "failed, reason {}",
            strerror(errno)
        );
        int is_non_blocking = fcntl(
            conn_fd,
            F_SETFL,
            saved_flags & ~O_NONBLOCK
        );
        AbortIfV(
            is_non_blocking == -1,
            "attempt to modify the connection file "
            "descriptor "
            "failed, reason {}",
            strerror(errno)
        );

        futures.push_back(std::async(
            std::launch::async,
            conn_handler_t{conn_fd, addr}
        ));
    }

    for (auto& future : futures) {
        // maybe we should use wait_for()? Just
        // in case something hangs?
        future.wait();
    }
}

}  // namespace server
