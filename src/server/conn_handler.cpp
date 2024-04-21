// server
#include "serial.hpp"
#include "threading.hpp"
#include <conn_handler.hpp>

// unix
#include <arpa/inet.h>
#include <fmt/format.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <unistd.h>

// share
#include <share.hpp>

// prot
#include <protocol.hpp>

// utils
#include <utils.hpp>
#include <variant>

#define MINUTE (60000)

namespace server {

conn_handler_t::conn_handler_t(
    int conn_fd, sockaddr_in addr)
    : m_conn_fd(conn_fd)
    , m_addr(addr)
{
}

void conn_handler_t::setup_readable_net_info()
{
    char ipv4[INET_ADDRSTRLEN + 1];

    if (inet_ntop(AF_INET, &m_addr.sin_addr, ipv4,
            INET_ADDRSTRLEN)
        == nullptr) {
        AbortV("inet_ntop(...) failed, reason: {}",
            strerror(errno));
    }

    m_ipv4 = std::string { ipv4 };

    m_port = ntohs(m_addr.sin_port);
}

void conn_handler_t::operator()()
{
    setup_readable_net_info();

    setup_logging(m_ipv4, m_port);

    log.info(
        "received a connection from {}:{}", m_ipv4, m_port);

    send_full_list();

    for (;;) {
        pollfd poll_fd = { m_conn_fd, POLLIN, 0 };

        if (poll(&poll_fd, 1, 10 * MINUTE) == -1) {
            log.error(
                "poll of connection failed, reason: {}",
                strerror(errno));

            break;
        }

        log.debug("wake up reasons {}, {}, {}",
            (poll_fd.revents & POLLIN) ? "POLLIN" : "",
            (poll_fd.revents & POLLHUP) ? "POLLHUP" : "",
            (poll_fd.revents & POLLERR) ? "POLLERR" : "");

        if (poll_fd.revents & POLLHUP) {
            log.info("closing tcp connection");

            break;
        }

        if (!(poll_fd.revents & POLLIN)) {
            log.info("server initiated closing");

            break;
        }

        prot::header_t header {};

        ssize_t header_size = read(
            m_conn_fd, &header, sizeof(prot::header_t));

        if (header_size == -1) {
            log.warn("reading from connection failed, "
                     "reason: {}",
                strerror(errno));

            continue;
        }

        if (header_size == 0) {
            // we will assume that if header_size == 0 and
            // the poll returns POLLIN that the write end of
            // the connection was close
            log.warn("input of length 0 bytes, hence "
                     "client closed write");

            break;
        }

        if (static_cast<size_t>(header_size)
            < sizeof(prot::header_t)) {
            log.warn("smallest possible read is {} bytes "
                     "instead got {} bytes",
                sizeof(prot::header_t), header_size);

            continue;
        }

        if (header.is_malformed()) {
            log.warn(
                "expected header start {} instead got {}",
                MAGIC_BYTES, header.magic_bytes);

            continue;
        }

        log.debug("expected payload size is {} bytes",
            header.payload_size);

        util::byte_vector payload { header.payload_size };

        ssize_t actual_size = read(
            m_conn_fd, payload.data(), header.payload_size);

        if (actual_size == -1) {
            log.warn(
                "continued reading from connection failed, "
                "reason: {}",
                strerror(errno));

            continue;
        }

        if (static_cast<size_t>(actual_size)
            < header.payload_size) {
            log.warn("header_size of payload ({} bytes) is "
                     "smaller than expected "
                     "({} bytes)",
                actual_size, header.payload_size);

            continue;
        }

        try {
            handle_payload(payload);
        } catch (util::serial_error_t& err) {
            log.warn("handling payload failed, reason: {}",
                err.what());
        } catch (std::runtime_error& err) {
            log.warn("handling payload failed, reason: {}",
                err.what());
        }

        log.flush();
    }

    log.info("closing connection handler");
}

void conn_handler_t::dtor()
{
    // make sure to update the e_connections that is, remove
    // the connection from e_connections because the updater
    // will still try to send to the connection (technically
    // it still shouldn't fail it just gives us an error but
    // that's no good)
    {
        threading::mutex_guard guard {
            share::e_connections_mutex
        };

        if (shutdown(m_conn_fd, SHUT_WR) == -1) {
            log.error("connection shutdown failed, "
                      "reason: {}",
                strerror(errno));
        }

        if (close(m_conn_fd) == -1) {
            AbortV("closing connection failed, reason: {}",
                strerror(errno));
        }

        share::e_connections.erase(m_conn_fd);
    }

    log.flush();
}

void conn_handler_t::send_full_list()
{
    // read the full list. again note this blocks
    prot::tagged_draw_list_t tagged_draws
        = share::e_draw_list.read();

    // serialize the list
    prot::serialize_t serializer { tagged_draws };

    util::byte_vector payload = serializer.bytes();

    // setup the header
    prot::header_t header { MAGIC_BYTES, payload.size() };

    // build the full packet
    util::byte_vector packet {};

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
        log.error("poll of connection failed, reason: {}",
            strerror(errno));

        return;
    }

    if (conn_poll.revents & POLLHUP) {
        log.error("connection hung up");

        return;
    }

    if (write(m_conn_fd, packet.data(), packet.size())
        == -1) {
        log.error(
            "writing to connection failed, reason: {}",
            strerror(errno));

        return;
    }
}

void conn_handler_t::handle_payload(
    const util::byte_vector& payload)
{
    prot::deserialize_t deserialize { payload };

    prot::payload_t payload_object {
        deserialize.payload()
    };

    if (!std::holds_alternative<prot::tagged_command_t>(
            payload_object)) {
        throw std::runtime_error(
            "does not contain a tagged command");
    }

    // NOTE: if we block on the push queue we might actually
    // miss data or overflow data that is sent to us in
    // buffer. That's what we are doing right now
    share::e_command_queue.push_back(
        std::get<prot::tagged_command_t>(payload_object));
    share::e_draw_list.update(
        std::get<prot::tagged_command_t>(payload_object));
}

logging::log conn_handler_t::log {};

void conn_handler_t::setup_logging(
    std::string ipv4, uint16_t port)
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix(fmt::format("[{}:{}]", ipv4, port));
}

} // namespace server
