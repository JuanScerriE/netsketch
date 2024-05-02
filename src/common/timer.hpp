#pragma once

// cstd
#include <cerrno>
#include <csignal>
#include <cstring>
#include <ctime>

// std
#include <stdexcept>

namespace server {

class Timer {
   public:
    Timer() = default;

    Timer(const Timer& other) = delete;

    Timer& operator=(const Timer& other) = delete;

    Timer(clockid_t clockid, sigevent* evp)
    {
        create(clockid, evp);
    }

    void create(clockid_t clockid, sigevent* evp)
    {
        if (timer_create(clockid, evp, &m_timer) == -1) {
            throw std::runtime_error { strerror(errno) };
        }
    }

    void
    set(int flags, const struct itimerspec* newval, struct itimerspec* oldval)
    {
        if (timer_settime(m_timer, flags, newval, oldval) == -1) {
            throw std::runtime_error { strerror(errno) };
        }
    }

    ~Timer() noexcept(false)
    {
        if (timer_delete(m_timer) == -1) {
            throw std::runtime_error { strerror(errno) };
        }
    }

   private:
    timer_t m_timer {};
};

// struct TimerData {
//     Timer timer {};
//
//     std::string user {};
// };

} // namespace server
