// client
#include <writer.hpp>

// share
#include <share.hpp>

// common
#include <serialization.hpp>
#include <types.hpp>

// unix
#include <poll.h>

// fmt
#include <fmt/core.h>
#include <fmt/format.h>

namespace client {

writer_t::writer_t(int conn_fd)
    : m_conn_fd(conn_fd)
{
}

common::message_t writer_t::get_message()
{
    common::draw_t msg
        = share::e_writer_queue.pop_back(); // wait

    common::serialized_t serialised_msg
        = common::serialize(msg);

    common::message_t message
        = common::pack(serialised_msg);

    return message;
}

void writer_t::operator()()
{
    for (;;) {
        common::message_t message = get_message();

        pollfd conn_poll { m_conn_fd, POLLOUT, 0 };

        if (poll(&conn_poll, 1, -1) == -1) {
            log::error(
                "poll of connection failed, reason: {}",
                strerror(errno));

            return;
        }

        if (conn_poll.revents & POLLHUP) {
            log::error("connection hung up");

            return;
        }

        if (write(m_conn_fd, message.data(), message.size())
            == -1) {
            log::error(
                "writing to connection failed, reason: {}",
                strerror(errno));

            return;
        }
    }
}

} // namespace client
