// client
#include <reader.hpp>

// common
#include <log.hpp>

// share
#include <share.hpp>

// fmt
#include <fmt/chrono.h>
#include <fmt/core.h>

// unix
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

namespace client {

reader_t::reader_t(int conn_fd)
    : m_conn_fd(conn_fd) {
}

void reader_t::operator()() {
    handle_loop();
}

void reader_t::handle_loop() {
    constexpr nfds_t nfds{2};

    constexpr size_t socket_idx{0};
    constexpr size_t event_idx{1};

    for (;;) {
        pollfd poll_fds[nfds] = {
            {m_conn_fd, POLLIN, 0},
            {share::e_stop_event->read_fd(), POLLIN, 0},
        };

        if (poll(poll_fds, nfds, -1) == -1) {
            log::error(
                "poll of connection failed, reason: {}",
                strerror(errno)
            );

            break;
        }

        log::debug(
            "wake up reasons Conn({}, {}, {}), Stop({}, "
            "{}, {})",
            (poll_fds[socket_idx].revents & POLLIN)
                ? "POLLIN"
                : "",
            (poll_fds[socket_idx].revents & POLLHUP)
                ? "POLLHUP"
                : "",
            (poll_fds[socket_idx].revents & POLLERR)
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

        if (poll_fds[socket_idx].revents & POLLHUP) {
            log::info("closing tcp connection");

            break;
        }

        if (poll_fds[event_idx].revents & POLLIN) {
            log::info("initiated closing");

            if (shutdown(m_conn_fd, SHUT_WR) == -1) {
                log::error(
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno)
                );
            }

            // NOTE: waiting on POLLHUP to actually exit
            // seems to be a bad idea since by the time
            // actually get a POLLHUP we would have
            // iterated and checked the event mutltiple
            // times.
            break;
        }

        AbortIf(
            !(poll_fds[socket_idx].revents & POLLIN),
            "expected POLLIN"
        );

        char buf[1024];

        bzero(buf, 1024);

        ssize_t size = read(m_conn_fd, buf, 1024 - 1);

        if (size == -1) {
            log::warn(
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
            log::warn(
                "input of length 0 bytes (server closed "
                "write)"
            );

            if (shutdown(m_conn_fd, SHUT_WR) == -1) {
                log::error(
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno)
                );
            }

            continue;
        }

        log::info("response: {}", buf);
    }
}

}  // namespace client
