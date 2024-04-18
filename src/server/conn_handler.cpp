// server
#include <conn_handler.hpp>

// unix
#include <arpa/inet.h>
#include <fmt/format.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

// share
#include <share.hpp>

// prot
#include <protocol.hpp>

// utils
#include <utils.hpp>

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

    log::set_level(log::level::debug);

    log::info(
        "received a connection from {}:{}", m_ipv4, m_port);

    constexpr nfds_t nfds { 2 };

    constexpr size_t conn_idx { 0 };
    constexpr size_t event_idx { 1 };

    for (;;) {
        pollfd poll_fds[nfds] = {
            { m_conn_fd, POLLIN, 0 },
            { share::e_stop_event->read_fd(), POLLIN, 0 },
        };

        if (poll(poll_fds, nfds, 10 * MINUTE) == -1) {
            addr_log(log::level::error,
                "poll of connection failed, reason: {}",
                strerror(errno));

            break;
        }

        addr_log(log::level::debug,
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
                : "");

        if (poll_fds[conn_idx].revents & POLLHUP) {
            addr_log(
                log::level::info, "closing tcp connection");

            break;
        }

        if (!(poll_fds[conn_idx].revents & POLLIN)
            || poll_fds[event_idx].revents & POLLIN) {
            addr_log(log::level::info,
                "server initiated closing");

            if (shutdown(m_conn_fd, SHUT_WR) == -1) {
                log::error(
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno));
            }

            break;
        }

        prot::header_t header {};

        ssize_t header_size = read(
            m_conn_fd, &header, sizeof(prot::header_t));

        if (header_size == -1) {
            addr_log(log::level::warn,
                "reading from connection failed, reason: "
                "{}",
                strerror(errno));

            continue;
        }

        if (header_size == 0) {
            // we will assume that if header_size == 0 and
            // the poll returns POLLIN that the write end of
            // the connection was close
            addr_log(log::level::warn,
                "input of length 0 bytes, hence client "
                "closed write");

            if (shutdown(m_conn_fd, SHUT_WR) == -1) {
                addr_log(log::level::error,
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno));
            }

            continue;
        }

        if (static_cast<size_t>(header_size)
            < sizeof(prot::header_t)) {
            addr_log(log::level::warn,
                "smallest possible read is {} bytes "
                "instead got {} bytes",
                sizeof(prot::header_t), header_size);

            continue;
        }

        if (header.is_malformed()) {
            addr_log(log::level::warn,
                "expected header start {} instead got {}",
                MAGIC_BYTES, header.magic_bytes);

            continue;
        }

        addr_log(log::level::debug,
            "expected payload size si {} bytes",
            header.payload_size);

        util::byte_vector payload { header.payload_size };

        ssize_t actual_size = read(
            m_conn_fd, payload.data(), header.payload_size);

        if (actual_size == -1) {
            addr_log(log::level::warn,
                "continued reading from connection failed, "
                "reason: {}",
                strerror(errno));

            continue;
        }

        if (static_cast<size_t>(actual_size)
            < header.payload_size) {
            addr_log(log::level::warn,
                "header_size of payload ({} bytes) is "
                "smaller than expected "
                "({} bytes)",
                actual_size, header.payload_size);

            continue;
        }

        handle_payload(payload);
    }

    addr_log(
        log::level::info, "closing connection handler");

    if (close(m_conn_fd) == -1) {
        AbortV("closing connection failed, reason: {}",
            strerror(errno));
    }
}

void conn_handler_t::handle_payload(
    const util::byte_vector& payload)
{
    prot::deserialize_t deserialize { payload };

    prot::payload_t payload_object {
        deserialize.payload()
    };
}

} // namespace server
