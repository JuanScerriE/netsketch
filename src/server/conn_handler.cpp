// server
#include <conn_handler.hpp>

// unix
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

// share
#include <share.hpp>

#define MINUTE (60000)

namespace server {
conn_handler_t::conn_handler_t(
    int conn_fd,
    sockaddr_in addr
)
    : m_conn_fd(conn_fd), m_addr(addr) {
}

void conn_handler_t::operator()() {
    setup_readable_net_info();

    log::set_level(log::level::debug);

    log::info(
        "received a connection from {}:{}",
        m_ipv4,
        m_port
    );

    constexpr nfds_t nfds{2};

    constexpr size_t conn_idx{0};
    constexpr size_t event_idx{1};

    for (;;) {
        pollfd poll_fds[nfds] = {
            {m_conn_fd, POLLIN, 0},
            {share::e_stop_event->read_fd(), POLLIN, 0},
        };

        if (poll(poll_fds, nfds, MINUTE) == -1) {
            addr_log(
                log::level::error,
                "poll of connection failed, reason: {}",
                strerror(errno)
            );

            break;
        }

        addr_log(
            log::level::debug,
            "wake up reasons Socket({}, {}, {}), Event({}, "
            "{}, {})",
            (poll_fds[conn_idx].revents & POLLIN) ? "POLLIN"
                                                  : "",
            (poll_fds[conn_idx].revents & POLLHUP)
                ? "POLLHUP"
                : "",
            (poll_fds[conn_idx].revents & POLLERR)
                ? "POLLERR"
                : "",
            (poll_fds[event_idx].revents & POLLIN)
                ? "POLLIN"
                : "",
            (poll_fds[event_idx].revents & POLLHUP)
                ? "POLLHUP"
                : "",
            (poll_fds[event_idx].revents & POLLERR)
                ? "POLLERR"
                : ""
        );

        if (poll_fds[conn_idx].revents & POLLHUP) {
            addr_log(
                log::level::info,
                "closing tcp connection"
            );

            break;
        }

        if (!(poll_fds[conn_idx].revents & POLLIN) || poll_fds[event_idx].revents & POLLIN) {
            addr_log(
                log::level::info,
                "server initiated closing"
            );

            if (shutdown(m_conn_fd, SHUT_WR) == -1) {
                log::error(
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno)
                );
            }

            break;
        }

        char buf[1024];

        bzero(buf, 1024);

        ssize_t size = read(m_conn_fd, buf, 1024 - 1);

        if (size == -1) {
            addr_log(
                log::level::warn,
                "reading from connection failed, reason: "
                "{}",
                strerror(errno)
            );

            continue;
        }

        if (size == 0) {
            // we will assume that if size == 0 and the poll
            // returns POLLIN that the write end of the
            // connection was close
            addr_log(
                log::level::warn,
                "input of length 0 bytes, hence client "
                "closed write"
            );

            if (shutdown(m_conn_fd, SHUT_WR) == -1) {
                addr_log(
                    log::level::error,
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno)
                );
            }

            continue;
        }

        addr_log(log::level::info, "response: {}", buf);
    }

    addr_log(
        log::level::info,
        "closing connection handler"
    );

    if (close(m_conn_fd) == -1) {
        AbortV(
            "closing connection failed, reason: {}",
            strerror(errno)
        );
    }
}

void conn_handler_t::setup_readable_net_info() {
    char ipv4[INET_ADDRSTRLEN + 1];

    if (inet_ntop(
            AF_INET,
            &m_addr.sin_addr,
            ipv4,
            INET_ADDRSTRLEN
        ) == nullptr) {
        AbortV(
            "inet_ntop(...) failed, reason: {}",
            strerror(errno)
        );
    }

    m_ipv4 = std::string{ipv4};

    m_port = ntohs(m_addr.sin_port);
}

}  // namespace server
