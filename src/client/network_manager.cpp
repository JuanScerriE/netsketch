// client
#include <network_manager.hpp>
#include <reader.hpp>
#include <writer.hpp>

// common
#include <log.hpp>
#include <log_file.hpp>
#include <share.hpp>

// fmt
#include <fmt/chrono.h>
#include <fmt/core.h>

// unix (hopefully)
#include <netinet/in.h>
#include <poll.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

namespace client {

network_manager_t::network_manager_t(
    uint32_t ipv4_addr, uint16_t port)
    : m_ipv4_addr(ipv4_addr)
    , m_port(port)
{
}

void network_manager_t::operator()()
{
    // construction
    if (!setup_connection()) {
        return;
    }
    setup_writer_thread();

    reader_t reader { m_conn_fd };

    reader();

    close_writer_thread();
    close_connection();
}

bool network_manager_t::setup_connection()
{
    m_conn_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (m_conn_fd == -1) {
        log::error("could not create socket, reason: {}",
            strerror(errno));

        return false;
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(m_ipv4_addr);
    server_addr.sin_port = htons(m_port);

    if (connect(m_conn_fd, (sockaddr*)&server_addr,
            sizeof(server_addr))
        == -1) {
        log::error("could not connect, reason: {}",
            strerror(errno));

        if (close(m_conn_fd) == -1) {
            AbortV("closing connection failed, reason: {}",
                strerror(errno));
        }

        return false;
    }

    return true;
}

void network_manager_t::close_connection()
{
    if (close(m_conn_fd) == -1) {
        AbortV("closing connection failed, reason: {}",
            strerror(errno));
    }
}

void network_manager_t::setup_writer_thread()
{
    m_writer_thread
        = std::thread { writer_t { m_conn_fd } };
}

void network_manager_t::close_writer_thread()
{
    m_writer_thread.join();
}

} // namespace client
