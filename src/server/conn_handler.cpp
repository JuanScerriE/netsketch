// server
#include <conn_handler.hpp>

// unix
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

#define MINUTE (60000)

namespace server {
conn_handler_t::conn_handler_t(
    int conn_fd,
    sockaddr_in addr,
    common::logger_t& logger
)
    : m_conn_fd(conn_fd), m_addr(addr), m_logger(logger) {
}

void conn_handler_t::operator()() {
    using level = common::level_e;

    char ip[INET_ADDRSTRLEN + 1];

    if (inet_ntop(
            AF_INET,
            &m_addr.sin_addr,
            ip,
            INET_ADDRSTRLEN
        ) == nullptr) {
        // we are expecting this to always work
        AbortV(
            "warn: convert network readable ip "
            "address to text failed, reason {}",
            strerror(errno)
        );
    }

    m_logger.log(
        level::INFO,
        "received a connection from {}:{}",
        ip,
        ntohs(m_addr.sin_port)
    );

    // set the human readable network info
    m_ipv4 = std::string{ip};
    m_port = ntohs(m_addr.sin_port);

    {  // this whole this is a test
        // we are assuming it is ready to write to
        ssize_t size =
            write(m_conn_fd, "hello", strlen("hello"));

        if (size == -1) {
            slog(
                level::ERROR,
                "writing to connection "
                "socket failed, reason {}",
                strerror(errno)
            );

            close(m_conn_fd);

            return;
        }
    }
#undef _GNU_SOURCE

    for (;;) {
#ifdef _GNU_SOURCE
        pollfd poll_fd{m_conn_fd, POLLIN | POLLRDHUP, 0};
#else
        pollfd poll_fd{m_conn_fd, POLLIN, 0};
#endif

        if (poll(&poll_fd, 1, MINUTE) == -1) {
            slog(
                level::ERROR,
                "poll of connection failed, reason: {}",
                strerror(errno)
            );

            break;
        }

        slog(
            level::DEBUG,
#ifdef _GNU_SOURCE
            "wake up reasons ({}, {}, {}, {})",
            (poll_fd.revents & POLLIN) ? "POLLIN" : "",
            (poll_fd.revents & POLLHUP) ? "POLLHUP" : "",
            (poll_fd.revents & POLLERR) ? "POLLERR" : "",
            (poll_fd.revents & POLLRDHUP) ? "POLLRDHUP" : ""
#else
            "wake up reasons ({}, {}, {}, {})",
            (poll_fd.revents & POLLIN) ? "POLLIN" : "",
            (poll_fd.revents & POLLHUP) ? "POLLHUP" : "",
            (poll_fd.revents & POLLERR) ? "POLLERR" : ""
#endif
        );

        if (poll_fd.revents & POLLHUP) {
            slog(level::INFO, "closing TCP connection");

            break;
        }

#ifdef _GNU_SOURCE
        if (poll_fd.revents & POLLRDHUP) {
            slog(level::INFO, "user initiated closing");

            // having shutdown both connections
            // poll with get POLLHUP
            shutdown(m_conn_fd, SHUT_RDWR);

            continue;
        }
#endif

#define _GNU_SOURCE
        if (!(poll_fd.revents & POLLIN)) {
            slog(level::INFO, "server initiated closing");

            break;
        }

        AbortIf(
            !(poll_fd.revents & POLLIN),
            "revents should have returned POLLIN"
        );

        char buf[1024];

        bzero(buf, 1024);

        if (read(m_conn_fd, buf, 1024 - 1) == -1) {
            slog(
                level::WARN,
                "reading from connection failed, reason: "
                "{}",
                strerror(errno)
            );

            continue;
        }

        if (strlen(buf) == 0) {
            if (is_still_alive()) {
                continue;
            }

            break;
        }

        slog(level::INFO, "response: {}", buf);
    }

    // TODO: make sure this correct
    close(m_conn_fd);
}

bool conn_handler_t::is_still_alive() {
    using level = common::level_e;

    pollfd maybe_dead_poll{m_conn_fd, POLLOUT, 0};

    if (poll(&maybe_dead_poll, 1, MINUTE / 2) == -1) {
        slog(
            level::WARN,
            "poll for potentially dead connection failed, "
            "reason: {}",
            strerror(errno)
        );

        return false;
    }

    if (!(maybe_dead_poll.revents & POLLOUT)) {
        slog(
            level::WARN,
            "poll interrupted and cannot write"
        );

        return false;
    }

    ssize_t size = write(
        m_conn_fd,
        "Are you alive?",
        strlen("Are you alive?")
    );

    if (size == -1) {
        slog(level::WARN, "cannot write to connection");

        return false;
    }

    maybe_dead_poll = {m_conn_fd, POLLIN, 0};

    if (poll(&maybe_dead_poll, 1, MINUTE / 2) == -1) {
        slog(
            level::WARN,
            "poll of potentially dead connection failed, "
            "reason: {}",
            strerror(errno)
        );

        return false;
    }

    if (!(maybe_dead_poll.revents & POLLIN)) {
        slog(level::WARN, "poll timeout");

        return false;
    }

    char buf[1024];

    bzero(buf, 1024);

    if (read(m_conn_fd, buf, 1024 - 1) == -1) {
        slog(level::WARN, "cannot read from connection");

        return false;
    }

    if (std::string{buf} != "Alive and kickin'") {
        return false;
    }

    return true;
}

}  // namespace server
