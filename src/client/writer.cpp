// client
#include "protocol.hpp"
#include <utils.hpp>
#include <writer.hpp>

// share
#include <share.hpp>

// common
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

util::byte_vector writer_t::get_message(
    prot::tagged_command_t& tagged_command)
{
    prot::serialize_t serializer { tagged_command };

    util::byte_vector payload_bytes = serializer.bytes();

    return payload_bytes;
}

void writer_t::operator()()
{
    for (;;) {
        // wait for tagged_command
        prot::tagged_command_t tagged_command
            = share::e_writer_queue.pop_back();

        // serialize the command
        auto payload = get_message(tagged_command);

        // setup the header
        prot::header_t header { MAGIC_BYTES,
            payload.size() };

        // build the full packet
        util::byte_vector packet {};

        packet.reserve(sizeof(header) + payload.size());

        for (auto& byte : util::to_bytes(header)) {
            packet.push_back(byte);
        }

        for (auto& byte : payload) {
            packet.push_back(byte);
        }

        log::debug("payload size {}", payload.size());

        log::debug("packet size {}", packet.size());

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

        if (write(m_conn_fd, packet.data(), packet.size())
            == -1) {
            log::error(
                "writing to connection failed, reason: {}",
                strerror(errno));

            return;
        }
    }
}

} // namespace client
