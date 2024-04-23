#pragma once

// unix
#include <fcntl.h>
#include <unistd.h>

// sys
#include <sys/socket.h>

// netinet
#include <netinet/in.h>

// std
#include <stdexcept>

// cstd
#include <cerrno>
#include <cstring>

// fmt
#include <fmt/core.h>

// abort
#include <abort.hpp>

class Socket {
public:
    Socket() = default;

    void open(int domain, int type, int protocol)
    {
        AbortIf(domain != AF_INET, "only supports AF_INET");

        m_domain = domain;
        m_type = type;
        m_protocol = protocol;

        std::memset(&m_sock_addr, 0, sizeof(m_sock_addr));

        m_sock_fd = socket(domain, type, protocol);

        if (m_sock_fd == -1) {
            throw std::runtime_error { fmt::format(
                "socket(): {}", strerror(errno)) };
        }
    }

    void make_blocking() const
    {
        AbortIf(m_sock_fd == -1, "uninitialized socket");

        int flags = fcntl(m_sock_fd, F_GETFL);
        AbortIfV(
            flags == -1, "fcntl(): {}", strerror(errno));
        int ret = fcntl(
            m_sock_fd, F_SETFL, flags & ~O_NONBLOCK);
        AbortIfV(ret == -1, "fcntl(): {}", strerror(errno));
    }

    void make_non_blocking() const
    {
        AbortIf(m_sock_fd == -1, "uninitialized socket");

        int flags = fcntl(m_sock_fd, F_GETFL);
        AbortIfV(
            flags == -1, "fcntl(): {}", strerror(errno));
        int ret
            = fcntl(m_sock_fd, F_SETFL, flags & O_NONBLOCK);
        AbortIfV(ret == -1, "fcntl(): {}", strerror(errno));
    }

    void bind(const struct sockaddr* sock_addr,
        socklen_t addr_size)
    {
        AbortIf(m_sock_fd == -1, "uninitialized socket");

        std::memcpy(&m_sock_addr, sock_addr, addr_size);

        m_addr_size = addr_size;

        int ret
            = ::bind(m_sock_fd, &m_sock_addr, m_addr_size);

        if (ret == -1) {
            throw std::runtime_error { fmt::format(
                "bind(): {}", strerror(errno)) };
        }
    }

    void listen(int backlog) const
    {
        AbortIf(m_sock_fd == -1, "uninitialized socket");

        int ret = ::listen(m_sock_fd, backlog);

        if (ret == -1) {
            throw std::runtime_error { fmt::format(
                "listen(): {}", strerror(errno)) };
        }
    }

    Socket accept(struct sockaddr* sock_addr,
        socklen_t* addr_size) const
    {
        AbortIf(m_sock_fd == -1, "uninitialized socket");

        int conn_sock_fd
            = ::accept(m_sock_fd, sock_addr, addr_size);

        if (conn_sock_fd == -1) {
            throw std::runtime_error { fmt::format(
                "accept(): {}", strerror(errno)) };
        }

        Socket conn_sock {};

        conn_sock.m_domain = m_domain;
        conn_sock.m_type = m_type;
        conn_sock.m_protocol = m_protocol;
        conn_sock.m_sock_addr = *sock_addr;
        conn_sock.m_addr_size = *addr_size;
        conn_sock.m_sock_fd = conn_sock_fd;

        return conn_sock;
    }

    void close()
    {
        AbortIf(m_sock_fd == -1, "uninitialized socket");

        AbortIfV(::close(m_sock_fd) == -1, "close(): {}",
            strerror(errno));

        m_domain = 0;
        m_type = 0;
        m_protocol = 0;
        m_sock_addr = {};
        m_addr_size = {};
        m_sock_fd = -1;
    }

    ~Socket()
    {
        if (m_sock_fd != -1) {
            close();
        }
    }

private:
    int m_domain { 0 };
    int m_type { 0 };
    int m_protocol { 0 };
    struct sockaddr m_sock_addr { };
    socklen_t m_addr_size {};
    int m_sock_fd { -1 };
};
