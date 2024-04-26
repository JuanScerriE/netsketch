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

void writer_t::operator()()
{
    for (;;) {
        // wait for tagged_command
        prot::TaggedCommand tagged_command = share::writer_queue.pop_back();

        // serialize the command
        prot::serialize_t serializer { tagged_command };

        util::ByteVector payload = serializer.bytes();

        // setup the header
        prot::PayloadHeader header { MAGIC_BYTES, payload.size() };

        // build the full packet
        util::ByteVector packet {};

        packet.reserve(sizeof(header) + payload.size());

        for (auto& byte : util::to_bytes(header)) {
            packet.push_back(byte);
        }

        for (auto& byte : payload) {
            packet.push_back(byte);
        }

        log.debug("payload size {}", payload.size());

        log.debug("packet size {}", packet.size());

        pollfd conn_poll { m_conn_fd, POLLOUT, 0 };

        if (poll(&conn_poll, 1, -1) == -1) {
            log.error("poll of connection failed, reason: {}", strerror(errno));

            return;
        }

        if (conn_poll.revents & POLLHUP) {
            log.error("connection hung up");

            return;
        }

        if (write(m_conn_fd, packet.data(), packet.size()) == -1) {
            log.error(
                "writing to connection failed, reason: {}",
                strerror(errno)
            );

            return;
        }
    }
}

void writer_t::dtor()
{
}

logging::log writer_t::log {};

void writer_t::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix("[writer]");

    log.set_file(share::log_file);
}

} // namespace client
