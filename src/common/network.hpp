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

// netsketch
#include "../common/abort.hpp"
#include "../common/bytes.hpp"

class PollResult {
   public:
    PollResult() = default;

    PollResult(bool timed_out, int events)
        : m_timed_out(timed_out), m_events(events)
    {
    }

    [[nodiscard]] bool has_timed_out() const
    {
        return m_timed_out;
    }

    [[nodiscard]] int get_events() const
    {
        return m_events;
    }

    [[nodiscard]] bool is_in() const
    {
        return m_events & POLLIN;
    }

    [[nodiscard]] bool is_out() const
    {
        return m_events & POLLOUT;
    }

    [[nodiscard]] bool is_err() const
    {
        return m_events & POLLERR;
    }

    [[nodiscard]] bool is_hup() const
    {
        return m_events & POLLHUP;
    }

    [[nodiscard]] bool is_nval() const
    {
        return m_events & POLLNVAL;
    }

    [[nodiscard]] bool is_pri() const
    {
        return m_events & POLLPRI;
    }

   private:
    bool m_timed_out {};

    int m_events {};
};

class ReadResult {
   public:
    ReadResult() = default;

    ReadResult(bool eof, ByteVector bytes)
        : m_eof(eof), m_bytes(std::move(bytes))
    {
    }

    [[nodiscard]] bool is_eof()
    {
        m_checked = true;

        return m_eof;
    }

    [[nodiscard]] ByteVector get_bytes() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_bytes;
    }

   private:
    bool m_checked { false };

    bool m_eof {};

    ByteVector m_bytes {};
};

class IPv4Socket {
   public:
    IPv4Socket() = default;

    IPv4Socket(IPv4Socket&& other) noexcept
        : m_domain(std::exchange(other.m_domain, 0))
        , m_type(std::exchange(other.m_type, 0))
        , m_protocol(std::exchange(other.m_protocol, 0))
        , m_sock_addr(std::exchange(other.m_sock_addr, {}))
        , m_addr_size(std::exchange(other.m_addr_size, {}))
        , m_sock_fd(std::exchange(other.m_sock_fd, -1))
    {
    }

    IPv4Socket& operator=(IPv4Socket&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_domain = std::exchange(other.m_domain, 0);
        m_type = std::exchange(other.m_type, 0);
        m_protocol = std::exchange(other.m_protocol, 0);
        m_sock_addr = std::exchange(other.m_sock_addr, {});
        m_addr_size = std::exchange(other.m_addr_size, {});
        m_sock_fd = std::exchange(other.m_sock_fd, -1);

        return *this;
    }

    IPv4Socket(const IPv4Socket& other) = delete;

    IPv4Socket& operator=(const IPv4Socket& other) = delete;

    [[nodiscard]] int native_handle() const
    {
        return m_sock_fd;
    }

    bool operator==(const IPv4Socket& other) const
    {
        return m_sock_fd == other.m_sock_fd;
    }

    [[nodiscard]] const struct sockaddr_in& get_sockaddr_in() const
    {
        return m_sock_addr;
    }

    void open(int type, int protocol)
    {
        ABORTIF(m_sock_fd != -1, "initialized socket");

        m_domain = AF_INET;
        m_type = type;
        m_protocol = protocol;

        bzero(&m_sock_addr, sizeof(m_sock_addr));

        m_sock_fd = socket(m_domain, m_type, m_protocol);

        if (m_sock_fd == -1) {
            throw std::runtime_error {
                fmt::format("socket(): {}", strerror(errno))
            };
        }
    }

    void make_blocking() const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        int flags = fcntl(m_sock_fd, F_GETFL);
        ABORTIFV(flags == -1, "fcntl(): {}", strerror(errno));
        int ret = fcntl(m_sock_fd, F_SETFL, flags & ~O_NONBLOCK);
        ABORTIFV(ret == -1, "fcntl(): {}", strerror(errno));
    }

    void make_non_blocking() const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        int flags = fcntl(m_sock_fd, F_GETFL);
        ABORTIFV(flags == -1, "fcntl(): {}", strerror(errno));
        int ret = fcntl(m_sock_fd, F_SETFL, flags & O_NONBLOCK);
        ABORTIFV(ret == -1, "fcntl(): {}", strerror(errno));
    }

    void bind(const struct sockaddr_in* sock_addr)
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ABORTIF(sock_addr->sin_family != AF_INET, "expected AF_INET");

        std::memcpy(&m_sock_addr, sock_addr, sizeof(struct sockaddr_in));

        m_addr_size = sizeof(struct sockaddr_in);

        int ret
            = ::bind(m_sock_fd, (struct sockaddr*)&m_sock_addr, m_addr_size);

        if (ret == -1) {
            throw std::runtime_error {
                fmt::format("bind(): {}", strerror(errno))
            };
        }
    }

    void listen(int backlog) const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        int ret = ::listen(m_sock_fd, backlog);

        if (ret == -1) {
            throw std::runtime_error {
                fmt::format("listen(): {}", strerror(errno))
            };
        }
    }

    [[nodiscard]] IPv4Socket accept() const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        IPv4Socket conn_sock {};

        int conn_sock_fd = ::accept(
            m_sock_fd,
            (struct sockaddr*)&conn_sock.m_sock_addr,
            &conn_sock.m_addr_size
        );

        if (conn_sock_fd == -1) {
            throw std::runtime_error {
                fmt::format("accept(): {}", strerror(errno))
            };
        }

        ABORTIF(
            conn_sock.m_addr_size != sizeof(struct sockaddr_in),
            "expected an IPv4 connection"
        );

        conn_sock.m_domain = m_domain;
        conn_sock.m_type = m_type;
        conn_sock.m_protocol = m_protocol;

        conn_sock.m_sock_fd = conn_sock_fd;

        return conn_sock;
    }

    void connect(const struct sockaddr_in* sock_addr)
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ABORTIF(sock_addr->sin_family != AF_INET, "expected AF_INET");

        std::memcpy(&m_sock_addr, sock_addr, sizeof(struct sockaddr_in));

        m_addr_size = sizeof(struct sockaddr_in);

        int ret
            = ::connect(m_sock_fd, (struct sockaddr*)&m_sock_addr, m_addr_size);

        if (ret == -1) {
            throw std::runtime_error {
                fmt::format("connect(): {}", strerror(errno))
            };
        }
    }

    PollResult poll(std::initializer_list<short> events, int timeout = -1)
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        short request = 0;

        for (auto& event : events) {
            request |= event;
        }

        struct pollfd query = { m_sock_fd, request, 0 };

        int ret = ::poll(&query, 1, timeout);

        if (ret == -1) {
            throw std::runtime_error {
                fmt::format("poll(): {}", strerror(errno))
            };
        }

        if (ret == 0) {
            return { true, 0 };
        }

        return { false, query.revents };
    }

    [[nodiscard]] ReadResult read(size_t size) const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ByteVector bytes { size };

        ssize_t read_size = ::read(m_sock_fd, bytes.data(), size);

        if (read_size < 0) {
            throw std::runtime_error {
                fmt::format("read(): {}", strerror(errno))
            };
        }

        if (read_size == 0) {
            return { true, {} };
        }

        if (static_cast<size_t>(read_size) < size) {
            throw std::runtime_error { "read(): fewer bytes than expected" };
        }

        return { false, bytes };
    }

    void write(ByteVector bytes) const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ssize_t write_size = ::write(m_sock_fd, bytes.data(), bytes.size());

        if (write_size < 0) {
            throw std::runtime_error {
                fmt::format("write(): {}", strerror(errno))
            };
        }

        if (static_cast<size_t>(write_size) < bytes.size()) {
            throw std::runtime_error { "write(): fewer bytes than expected" };
        }
    }

    void close()
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ABORTIFV(::close(m_sock_fd) == -1, "close(): {}", strerror(errno));

        m_domain = 0;
        m_type = 0;
        m_protocol = 0;
        m_sock_addr = {};
        m_addr_size = {};
        m_sock_fd = -1;
    }

    ~IPv4Socket()
    {
        if (m_sock_fd != -1) {
            close();
        }
    }

    friend class IPv4SocketRef;

   private:
    int m_domain { 0 };
    int m_type { 0 };
    int m_protocol { 0 };
    struct sockaddr_in m_sock_addr { };
    socklen_t m_addr_size {};
    int m_sock_fd { -1 };
};

class IPv4SocketRef {
   public:
    IPv4SocketRef() = default;

    explicit IPv4SocketRef(IPv4Socket& other) noexcept
        : m_domain(other.m_domain)
        , m_type(other.m_type)
        , m_protocol(other.m_protocol)
        , m_sock_addr(other.m_sock_addr)
        , m_addr_size(other.m_addr_size)
        , m_sock_fd(other.m_sock_fd)
    {
    }

    IPv4SocketRef(const IPv4SocketRef& other) noexcept = default;

    IPv4SocketRef& operator=(const IPv4SocketRef& other) noexcept
    {
        if (this == &other)
            return *this;

        m_domain = other.m_domain;
        m_type = other.m_type;
        m_protocol = other.m_protocol;
        m_sock_addr = other.m_sock_addr;
        m_addr_size = other.m_addr_size;
        m_sock_fd = other.m_sock_fd;

        return *this;
    }

    [[nodiscard]] int native_handle() const
    {
        return m_sock_fd;
    }

    bool operator==(const IPv4SocketRef& other) const
    {
        return m_sock_fd == other.m_sock_fd;
    }

    [[nodiscard]] const struct sockaddr_in& get_sockaddr_in() const
    {
        return m_sock_addr;
    }

    PollResult poll(std::initializer_list<short> events, int timeout = -1)
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        short request = 0;

        for (auto& event : events) {
            request |= event;
        }

        struct pollfd query = { m_sock_fd, request, 0 };

        int ret = ::poll(&query, 1, timeout);

        if (ret == -1) {
            throw std::runtime_error {
                fmt::format("poll(): {}", strerror(errno))
            };
        }

        if (ret == 0) {
            return { true, 0 };
        }

        return { false, query.revents };
    }

    [[nodiscard]] ReadResult read(size_t size) const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ByteVector bytes { size };

        ssize_t read_size = ::read(m_sock_fd, bytes.data(), size);

        if (read_size < 0) {
            throw std::runtime_error {
                fmt::format("read(): {}", strerror(errno))
            };
        }

        if (read_size == 0) {
            return { true, {} };
        }

        if (static_cast<size_t>(read_size) < size) {
            throw std::runtime_error { "read(): fewer bytes than expected" };
        }

        return { false, bytes };
    }

    void write(ByteVector bytes) const
    {
        ABORTIF(m_sock_fd == -1, "uninitialized socket");

        ssize_t write_size = ::write(m_sock_fd, bytes.data(), bytes.size());

        if (write_size < 0) {
            throw std::runtime_error {
                fmt::format("write(): {}", strerror(errno))
            };
        }

        if (static_cast<size_t>(write_size) < bytes.size()) {
            throw std::runtime_error { "write(): fewer bytes than expected" };
        }
    }

   private:
    int m_domain { 0 };
    int m_type { 0 };
    int m_protocol { 0 };
    struct sockaddr_in m_sock_addr { };
    socklen_t m_addr_size {};
    int m_sock_fd { -1 };
};

template <>
struct std::hash<IPv4Socket> {
    std::size_t operator()(const IPv4Socket& sock) const noexcept
    {
        return static_cast<std::size_t>(sock.native_handle());
    }

    std::size_t operator()(const IPv4SocketRef& sock) const noexcept
    {
        return static_cast<std::size_t>(sock.native_handle());
    }
};

template <>
struct std::hash<IPv4SocketRef> {
    std::size_t operator()(const IPv4Socket& sock) const noexcept
    {
        return static_cast<std::size_t>(sock.native_handle());
    }

    std::size_t operator()(const IPv4SocketRef& sock) const noexcept
    {
        return static_cast<std::size_t>(sock.native_handle());
    }
};
