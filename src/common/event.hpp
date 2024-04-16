#pragma once

// unix
#include <unistd.h>

// std
#include <cerrno>
#include <cstring>

// common
#include <abort.hpp>

#define NOTIFY (1)

#ifdef __APPLE__
#define PIPE_SIZE (2)
#define READ (0)
#define WRITE (1)
#else
#include <sys/eventfd.h>
#endif

namespace common {

class event_t {
   public:
    event_t(const event_t&) = delete;
    event_t& operator=(const event_t&) = delete;

    event_t(event_t&& mE) = default;
    event_t& operator=(event_t&& mE) = default;

    explicit event_t() {
#ifdef __APPLE__
        if (pipe(m_stop_event_pipe) == -1) {
            AbortV(
                "failed to create pipe file descriptor, "
                "reason: {}",
                strerror(errno)
            );
        }
#else
        m_stop_event_fd = eventfd(0, 0);

        if (m_stop_event_fd == -1) {
            AbortV(
                "failed to create event file descriptor, "
                "reason: {}",
                strerror(errno)
            );
        }
#endif
    }

    ~event_t() {
#ifdef __APPLE__
        if (close(m_stop_event_pipe[READ]) == -1) {
            AbortV(
                "failed to close read end of a pipe, "
                "reason: {}",
                strerror(errno)
            );
        }

        if (close(m_stop_event_pipe[WRITE]) == -1) {
            AbortV(
                "failed to close write end of a pipe, "
                "reason: {}",
                strerror(errno)
            );
        }
#else
        if (close(m_stop_event_fd) == -1) {
            AbortV(
                "failed to close event file descriptor, "
                "reason: {}",
                strerror(errno)
            );
        }
#endif
    }

    void notify() {
        uint64_t notify{NOTIFY};

#ifdef __APPLE__
        if (write(
                m_stop_event_pipe[WRITE],
                &notify,
                sizeof(notify)
            ) == -1) {
            AbortV(
                "failed to write to pipe, reason: {}",
                strerror(errno)
            );
        }
#else
        if (write(
                m_stop_event_fd,
                &notify,
                sizeof(notify)
            ) == -1) {
            AbortV(
                "failed to write to event file descriptor, "
                "reason: {}",
                strerror(errno)
            );
        }
#endif
    }

    void reset() {
        uint64_t throw_away{};

#ifdef __APPLE__
        if (read(
                m_stop_event_pipe[READ],
                &throw_away,
                sizeof(throw_away)
            ) == -1) {
            AbortV(
                "failed to read from pipe, reason: {}",
                strerror(errno)
            );
        }
#else
        if (read(
                m_stop_event_fd,
                &throw_away,
                sizeof(throw_away)
            ) == -1) {
            AbortV(
                "failed to read from event file "
                "descriptor, "
                "reason: {}",
                strerror(errno)
            );
        }
#endif
    }

    [[nodiscard]] int read_fd() const {
#ifdef __APPLE__
        return m_stop_event_pipe[READ];
#else
        return m_stop_event_fd;
#endif
    }

    [[nodiscard]] int write_fd() const {
#ifdef __APPLE__
        return m_stop_event_pipe[WRITE];
#else
        return m_stop_event_fd;
#endif
    }

   private:
#ifdef __APPLE__
    int m_stop_event_pipe[PIPE_SIZE]{0, 0};
#else
    int m_stop_event_fd{0};
#endif
};

}  // namespace common
