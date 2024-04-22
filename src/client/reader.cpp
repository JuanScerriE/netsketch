// client
#include "draw_list.hpp"
#include "protocol.hpp"
#include "threading.hpp"
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
#include <sys/poll.h>
#include <unistd.h>
#include <variant>

namespace client {

reader_t::reader_t(int conn_fd)
    : m_conn_fd(conn_fd)
{
}

void reader_t::operator()()
{
    setup_logging();

    handle_loop();
}

void reader_t::handle_loop()
{
    for (;;) {
        pollfd poll_fd = { m_conn_fd, POLLIN, 0 };

        if (poll(&poll_fd, 1, -1) == -1) {
            log.error(
                "poll of connection failed, reason: {}",
                strerror(errno));

            break;
        }

        log.debug("wake up reasons ({}, {}, {})",
            (poll_fd.revents & POLLIN) ? "POLLIN" : "",
            (poll_fd.revents & POLLHUP) ? "POLLHUP" : "",
            (poll_fd.revents & POLLERR) ? "POLLERR" : "");

        if (poll_fd.revents & POLLHUP) {
            log.info("closing tcp connection");

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
            log.warn("input of length 0 bytes (server "
                     "closed write)");

            if (::shutdown(m_conn_fd, SHUT_WR) == -1) {
                log.error("connection shutdown failed, "
                          "reason: {}",
                    strerror(errno));
            }

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

        log.debug("payload size is {} bytes",
            header.payload_size);

        util::byte_vector payload { header.payload_size };

        ssize_t actual_size = read(
            m_conn_fd, payload.data(), header.payload_size);

        if (actual_size == -1) {
            log.warn("continued reading from connection "
                     "failed, reason: {}",
                strerror(errno));

            continue;
        }

        if (static_cast<size_t>(actual_size)
            < header.payload_size) {
            log.warn("header_size of payload ({} bytes) is "
                     "smaller than expected ({} bytes)",
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
    shutdown();
}

void reader_t::shutdown()
{
    if (share::writer_thread.is_initialized()
        && share::writer_thread.is_alive())
        share::writer_thread.cancel();
    if (share::input_thread.is_initialized()
        && share::input_thread.is_alive())
        share::input_thread.cancel();

    // make sure to stop the gui
    common::mutable_t<bool> { share::stop_gui }() = true;
}

void reader_t::dtor()
{
    log.info("initiated closing");

    if (::shutdown(m_conn_fd, SHUT_WR) == -1) {
        if (errno == ENOTCONN) {
            log.warn(
                "connection shutdown failed, reason: {}",
                strerror(errno));
        } else {
            log.error(
                "connection shutdown failed, reason: {}",
                strerror(errno));
        }
    }
}

// payload section
//
// (TODO: update the double buffer, cause currently
// it is not thread-safe and with enough context
// switching it will fail)

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

        return;
    }

    if (std::holds_alternative<prot::tagged_draw_t>(
            payload_object)) {
        auto& tagged_draw
            = std::get<prot::tagged_draw_t>(payload_object);
        (void)tagged_draw;

        return;
    }

    if (std::holds_alternative<prot::tagged_draw_list_t>(
            payload_object)) {
        auto& tagged_draw_list
            = std::get<prot::tagged_draw_list_t>(
                payload_object);

        update_whole_list(tagged_draw_list);

        return;
    }

    throw std::runtime_error("does not contain known type");
}

void reader_t::update_whole_list(
    prot::tagged_draw_list_t& list)
{
    threading::mutex_guard guard { share::writer_mutex };
    {
        threading::rwlock_wrguard wrguard {
            share::rwlock1
        };

        share::list1 = list;
    }

    {
        threading::rwlock_wrguard wrguard {
            share::rwlock2
        };

        share::list2 = list;
    }
}

void reader_t::update_list(
    prot::tagged_command_t& tagged_command)
{
    threading::mutex_guard guard { share::writer_mutex };
    {
        threading::rwlock_wrguard wrguard {
            share::rwlock1
        };

        common::draw_list_wrapper { share::list1 }.update(
            tagged_command);
    }

    {
        threading::rwlock_wrguard wrguard {
            share::rwlock2
        };

        common::draw_list_wrapper { share::list2 }.update(
            tagged_command);
    }
}

// end section

logging::log reader_t::log {};

void reader_t::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix("[reader]");

    log.set_file(share::log_file);
}

} // namespace client
