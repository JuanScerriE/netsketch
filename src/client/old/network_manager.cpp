// client
#include <network_manager.hpp>
#include <reader.hpp>
#include <writer.hpp>

// share
#include <share.hpp>

// logging
#include <log.hpp>

// unix (hopefully)
#include <netinet/in.h>
#include <poll.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

namespace client {

network_manager_t::network_manager_t(uint32_t ipv4_addr, uint16_t port)
    : m_ipv4_addr(ipv4_addr), m_port(port)
{
    setup_logging();
}

bool network_manager_t::setup()
{
    if (!setup_connection()) {
        return false;
    }

    setup_reader_thread();
    setup_writer_thread();

    return true;
}

void network_manager_t::close()
{
    close_writer_thread();
    close_reader_thread();
    close_connection();
}

bool network_manager_t::setup_connection()
{
    m_conn_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (m_conn_fd == -1) {
        log.error("could not create socket, reason: {}", strerror(errno));

        log.flush();

        return false;
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(m_ipv4_addr);
    server_addr.sin_port = htons(m_port);

    if (connect(m_conn_fd, (sockaddr*)&server_addr, sizeof(server_addr))
        == -1) {
        log.error("could not connect, reason: {}", strerror(errno));

        log.flush();

        if (::close(m_conn_fd) == -1) {
            ABORTV("closing connection failed, reason: {}", strerror(errno));
        }

        return false;
    }

    pollfd poll_fd = { m_conn_fd, POLLIN, 0 };

    if (poll(&poll_fd, 1, 60000) == -1) {
        log.error("poll of connection failed, reason: {}", strerror(errno));

        if (::close(m_conn_fd) == -1) {
            ABORTV("closing connection failed, reason: {}", strerror(errno));
        }

        return false;
    }

    if (poll_fd.revents & POLLHUP) {
        log.warn("connection hung up");

        if (::close(m_conn_fd) == -1) {
            ABORTV("closing connection failed, reason: {}", strerror(errno));
        }

        return false;
    }

    if (!(poll_fd.revents & POLLIN)) {
        log.warn("establishing connection timedout");

        if (::close(m_conn_fd) == -1) {
            ABORTV("closing connection failed, reason: {}", strerror(errno));
        }

        return false;
    }

    uint16_t check {};

    ssize_t check_size = read(m_conn_fd, &check, sizeof(check));

    if (check_size == -1) {
        log.warn(
            "reading from connection failed, "
            "reason: {}",
            strerror(errno)
        );

        if (::close(m_conn_fd) == -1) {
            ABORTV("closing connection failed, reason: {}", strerror(errno));
        }

        return false;
    }

    if (check_size != sizeof(check)) {
        log.warn("unexpected message size ({} bytes)", check_size);

        if (::close(m_conn_fd) == -1) {
            ABORTV("closing connection failed, reason: {}", strerror(errno));
        }

        return false;
    }

    uint16_t host_check = ntohs(check);

    // accepted
    if (host_check == 1) {
        log.info("connection established");

        return true;
    }

    // declined
    if (host_check == 2) {
        log.info("connection refused");

        if (::close(m_conn_fd) == -1) {
            ABORTV("closing connection failed, reason: {}", strerror(errno));
        }

        return false;
    }

    log.error("unexpected result");

    if (::close(m_conn_fd) == -1) {
        ABORTV("closing connection failed, reason: {}", strerror(errno));
    }

    return false;
}

void network_manager_t::close_connection()
{
    if (::close(m_conn_fd) == -1) {
        ABORTV("closing connection failed, reason: {}", strerror(errno));
    }
}

void network_manager_t::setup_reader_thread()
{
    share::reader_thread = threading::pthread { reader_t { m_conn_fd } };
}

void network_manager_t::close_reader_thread()
{
    if (share::reader_thread.is_initialized())
        share::reader_thread.join();
}

void network_manager_t::setup_writer_thread()
{
    share::writer_thread = threading::pthread { writer_t { m_conn_fd } };
}

void network_manager_t::close_writer_thread()
{
    if (share::writer_thread.is_initialized())
        share::writer_thread.join();
}

logging::log network_manager_t::log {};

void network_manager_t::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix("[network_manager]");

    log.set_file(share::log_file);
}

} // namespace client
