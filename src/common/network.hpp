#pragma once

// unix
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

// std
#include <stdexcept>
#include <utility>
#include <vector>

// cstd
#include <cerrno>
#include <cstring>

// fmt
#include <fmt/core.h>

// common
#include "abort.hpp"
#include "bytes.hpp"

// This is a wrapper class around the return result of a
// call to poll

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

// This is a wrapper class around the return result of a
// call to read

class ReadResult {
   public:
    ReadResult() = default;

    ReadResult(bool eof, ByteString bytes)
        : m_eof(eof), m_bytes(std::move(bytes))
    {
    }

    [[nodiscard]] bool is_eof()
    {
        m_checked = true;

        return m_eof;
    }

    [[nodiscard]] ByteString get_bytes() const
    {
        ABORTIF(!m_checked, "poll result not checked");

        return m_bytes;
    }

   private:
    bool m_checked { false };

    bool m_eof {};

    ByteString m_bytes {};
};

// This class is a wrapper around a socket file
// descriptor. This class helps us automatically
// close the socket through RAII. Additionally,
// it enforces a requirement for usage of IPv4
//
// A bunch of helper functions have been added
// to facilitate easy of use.

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

        conn_sock.m_addr_size = sizeof(struct sockaddr_in);

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

        std::vector<char> vec_bytes {};

        vec_bytes.reserve(size);

        ssize_t read_size = ::read(m_sock_fd, vec_bytes.data(), size);

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

        ByteString bytes {};

        bytes.reserve(size);

        for (size_t i = 0; i < size; i++) {
            bytes.push_back(vec_bytes[i]);
        }

        return { false, bytes };
    }

    void write(ByteString bytes) const
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

// This class is also a wrapper around C sockets however, the main point of this
// class is that it is a reference. So, it is capable of providing an interface
// for interacting with the socket. However, it cannot create nor destroy a
// socket. This class is meant to be easily copyable and hence it allows sharing
// of socket through the program as a value. Of course, one might argue why not
// just use a C++ reference Good point. However, in our situation we are keeping
// track of the number of open sockets in a unordered_map or vector. The problem
// with passing a reference to an object in vector especially to another thread,
// is that you can end up with a dangling reference since there is nothing from
// stopping the unordered_map or vector holding the actual object from
// reshuffling the objects or moving the objects around. This essentially, means
// that you'll be left with a dangling reference. Of course this is as bad a
// dangling pointer. However, the socket file descriptor itself lives for as
// long as the program decides to keep it open, is trivially copyable since it
// is an integer and is unique upto closure. Hence, we have the necessary
// guarantees for creating a reference object which will always reference what
// its suppose to reference (hence, why we create the IPv4SocketRef class).

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

        std::vector<char> vec_bytes {};

        vec_bytes.reserve(size);

        ssize_t read_size
            = ::recv(m_sock_fd, vec_bytes.data(), size, MSG_WAITALL);

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

        ByteString bytes {};

        bytes.reserve(size);

        for (size_t i = 0; i < size; i++) {
            bytes.push_back(vec_bytes[i]);
        }

        return { false, bytes };
    }

    void write(ByteString bytes) const
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

// Here we augment the standard std::hash to
// include the above classes. We can use the native
// handle (the file descriptor) since it is
// guaranteed to be unique, for the running
// process

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
