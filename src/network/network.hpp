#pragma once

// unix
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

// sys
#include <sys/socket.h>

// netinet
#include <netinet/in.h>

// std
#include <stdexcept>
#include <utility>

// cstd
#include <cerrno>
#include <cstring>

// fmt
#include <fmt/core.h>

// abort
#include <abort.hpp>

class PollResult {
public:
    PollResult(int error_no, bool timed_out, int events)
        : m_error_no(error_no)
        , m_timed_out(timed_out)
        , m_events(events)
    {
    }

    [[nodiscard]] bool has_error()
    {
        m_checked = true;

        return m_error_no != 0;
    }

    [[nodiscard]] int get_error() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_error_no;
    }

    [[nodiscard]] bool has_timed_out() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_timed_out;
    }

    [[nodiscard]] int get_events() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_events;
    }

    [[nodiscard]] bool is_in() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_events & POLLIN;
    }

    [[nodiscard]] bool is_out() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_events & POLLOUT;
    }

    [[nodiscard]] bool is_err() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_events & POLLERR;
    }

    [[nodiscard]] bool is_hup() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_events & POLLHUP;
    }

    [[nodiscard]] bool is_nval() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_events & POLLNVAL;
    }

    [[nodiscard]] bool is_pri() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_events & POLLPRI;
    }

private:
    bool m_checked { false };

    int m_error_no {};
    bool m_timed_out {};
    int m_events {};
};

class Socket {
public:
    Socket() = default;

    Socket(Socket&& other) noexcept
        : m_domain(std::exchange(other.m_domain, 0))
        , m_type(std::exchange(other.m_type, 0))
        , m_protocol(std::exchange(other.m_protocol, 0))
        , m_sock_addr(std::exchange(other.m_sock_addr, {}))
        , m_addr_size(std::exchange(other.m_addr_size, {}))
        , m_sock_fd(std::exchange(other.m_sock_fd, -1))
    {
    }

    Socket& operator=(Socket&& other) noexcept
    {
        m_domain = std::exchange(other.m_domain, 0);
        m_type = std::exchange(other.m_type, 0);
        m_protocol = std::exchange(other.m_protocol, 0);
        m_sock_addr = std::exchange(other.m_sock_addr, {});
        m_addr_size = std::exchange(other.m_addr_size, {});

        return *this;
    }

    void open(int domain, int type, int protocol)
    {
        ABORTIF(domain != AF_INET, "only supports AF_INET");

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
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        int flags = fcntl(m_sock_fd, F_GETFL);
        ABORTIFV(
            flags == -1, "fcntl(): {}", strerror(errno));
        int ret = fcntl(
            m_sock_fd, F_SETFL, flags & ~O_NONBLOCK);
        ABORTIFV(ret == -1, "fcntl(): {}", strerror(errno));
    }

    void make_non_blocking() const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        int flags = fcntl(m_sock_fd, F_GETFL);
        ABORTIFV(
            flags == -1, "fcntl(): {}", strerror(errno));
        int ret
            = fcntl(m_sock_fd, F_SETFL, flags & O_NONBLOCK);
        ABORTIFV(ret == -1, "fcntl(): {}", strerror(errno));
    }

    void bind(const struct sockaddr* sock_addr,
        socklen_t addr_size)
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

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
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        int ret = ::listen(m_sock_fd, backlog);

        if (ret == -1) {
            throw std::runtime_error { fmt::format(
                "listen(): {}", strerror(errno)) };
        }
    }

    template <typename... Events>
    PollResult poll(Events... events, int timeout = 128)
    {
        struct pollfd query
            = { m_sock_fd, (events | ...), 0 };

        int ret = ::poll(&query, 1, timeout);

        if (ret == -1) {
            return { errno, false, 0 };
        }

        if (ret == 0) {
            return { 0, true, 0 };
        }

        return { 0, false, query.revents };
    }

    [[nodiscard]] Socket accept() const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        Socket conn_sock {};

        int conn_sock_fd = ::accept(m_sock_fd,
            &conn_sock.m_sock_addr, &conn_sock.m_addr_size);

        if (conn_sock_fd == -1) {
            throw std::runtime_error { fmt::format(
                "accept(): {}", strerror(errno)) };
        }

        conn_sock.m_domain = m_domain;
        conn_sock.m_type = m_type;
        conn_sock.m_protocol = m_protocol;
        // conn_sock.m_sock_addr;
        // conn_sock.m_addr_size;
        conn_sock.m_sock_fd = conn_sock_fd;

        return conn_sock;
    }

    void close()
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ABORTIFV(::close(m_sock_fd) == -1, "close(): {}",
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
