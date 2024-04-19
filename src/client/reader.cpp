// client
#include "draw_list.hpp"
#include "protocol.hpp"
#include "utils.hpp"
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
#include <variant>

namespace client {

reader_t::reader_t(int conn_fd)
    : m_conn_fd(conn_fd)
{
}

void reader_t::operator()() { handle_loop(); }

void reader_t::handle_loop()
{
    constexpr nfds_t nfds { 2 };

    constexpr size_t socket_idx { 0 };
    constexpr size_t event_idx { 1 };

    for (;;) {
        pollfd poll_fds[nfds] = {
            { m_conn_fd, POLLIN, 0 },
            { share::e_stop_event->read_fd(), POLLIN, 0 },
        };

        if (poll(poll_fds, nfds, -1) == -1) {
            log::error(
                "poll of connection failed, reason: {}",
                strerror(errno));

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
                : "");

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
                    strerror(errno));
            }

            // NOTE: waiting on POLLHUP to actually exit
            // seems to be a bad idea since by the time
            // actually get a POLLHUP we would have
            // iterated and checked the event mutltiple
            // times.
            break;
        }

        AbortIf(!(poll_fds[socket_idx].revents & POLLIN),
            "expected POLLIN");

        prot::header_t header {};

        ssize_t header_size = read(
            m_conn_fd, &header, sizeof(prot::header_t));

        if (header_size == -1) {
            log::warn(
                "reading from connection failed, reason: "
                "{}",
                strerror(errno));

            continue;
        }

        if (header_size == 0) {
            // we will assume that if header_size == 0 and
            // the poll returns POLLIN that the write end of
            // the connection was close
            log::warn(
                "input of length 0 bytes (server closed "
                "write)");

            if (shutdown(m_conn_fd, SHUT_WR) == -1) {
                log::error(
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno));
            }

            continue;
        }

        if (static_cast<size_t>(header_size)
            < sizeof(prot::header_t)) {
            log::warn("smallest possible read is {} bytes "
                      "instead got {} bytes",
                sizeof(prot::header_t), header_size);

            continue;
        }

        if (header.is_malformed()) {
            log::warn(
                "expected header start {} instead got {}",
                MAGIC_BYTES, header.magic_bytes);

            continue;
        }

        log::debug("expected payload size is {} bytes",
            header.payload_size);

        util::byte_vector payload { header.payload_size };

        ssize_t actual_size = read(
            m_conn_fd, payload.data(), header.payload_size);

        if (actual_size == -1) {
            log::warn(
                "continued reading from connection failed, "
                "reason: {}",
                strerror(errno));

            continue;
        }

        if (static_cast<size_t>(actual_size)
            < header.payload_size) {
            log::warn(
                "header_size of payload ({} bytes) is "
                "smaller than expected "
                "({} bytes)",
                actual_size, header.payload_size);

            continue;
        }

        try {
            handle_payload(payload);
        } catch (util::serial_error_t& err) {
            log::warn("handling payload failed, reason: {}",
                err.what());
        } catch (std::runtime_error& err) {
            log::warn("handling payload failed, reason: {}",
                err.what());
        }
    }
}

void reader_t::handle_payload(util::byte_vector& payload)
{
    prot::deserialize_t deserializer { payload };

    prot::payload_t payload_object = deserializer.payload();

    if (std::holds_alternative<prot::tagged_command_t>(
            payload_object)) {
        auto& tagged_command
            = std::get<prot::tagged_command_t>(
                payload_object);
        update_list(tagged_command);
    }

    if (std::holds_alternative<prot::tagged_draw_t>(
            payload_object)) {
        auto& tagged_draw
            = std::get<prot::tagged_draw_t>(payload_object);
        (void)tagged_draw;
    }

    if (std::holds_alternative<prot::tagged_draw_list_t>(
            payload_object)) {
        auto& tagged_draw_list
            = std::get<prot::tagged_draw_list_t>(
                payload_object);

        update_whole_list(tagged_draw_list);
    }

    throw std::runtime_error("does not contain known type");
}

void reader_t::update_whole_list(
    prot::tagged_draw_list_t& list)
{
    // so we check if the current list is the first
    // in the case that it is we set the index
    // to the second.
    int index = 0;

    if (share::current_list == &share::lists[index]) {
        index = 1;
    }

    share::lists[index] = list;
    share::current_list = &share::lists[index];
    share::lists[1 - index] = share::lists[index];
}

void reader_t::update_list(
    prot::tagged_command_t& tagged_command)
{
    // so we check if the current list is the first
    // in the case that it is we set the index
    // to the second.
    int index = 0;

    if (share::current_list == &share::lists[index]) {
        index = 1;
    }

    common::draw_list_wrapper { share::lists[index] }
        .update(tagged_command);
    share::current_list = &share::lists[index];
    share::lists[1 - index] = share::lists[index];
}

} // namespace client
