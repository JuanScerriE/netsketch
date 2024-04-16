// client
#include <writer.hpp>

// share
#include <share.hpp>

// unix
#include <poll.h>

namespace client {

writer_t::writer_t(int conn_fd)
    : m_conn_fd(conn_fd) {
}

void writer_t::operator()() {
    for (;;) {
        std::string msg =
            share::e_writer_queue.pop_back();  // wait

        pollfd conn_poll{m_conn_fd, POLLOUT, 0};

        if (poll(&conn_poll, 1, -1) == -1) {
            log::error(
                "poll of connection failed, reason: {}",
                strerror(errno)
            );

            return;
        }

        if (conn_poll.revents & POLLHUP) {
            log::error("connection hung up");

            return;
        }

        if (write(m_conn_fd, msg.c_str(), msg.length()) ==
            -1) {
            log::error(
                "writing to connection failed, reason: {}",
                strerror(errno)
            );

            return;
        }
    }
}

}  // namespace client
