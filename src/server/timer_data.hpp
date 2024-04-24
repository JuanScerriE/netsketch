#pragma once

// std
#include <cerrno>
#include <csignal>
#include <cstring>
#include <stdexcept>

// unix
#include <ctime>

namespace server::timing {
class timer {
   public:
    timer() = default;

    timer(clockid_t clockid, sigevent* evp)
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
    set(int flags,
        const struct itimerspec* newval,
        struct itimerspec* oldval)
    {
        if (timer_settime(m_timer, flags, newval, oldval)
            == -1) {
            throw std::runtime_error { strerror(errno) };
        }
    }

    ~timer() noexcept(false)
    {
        if (timer_delete(m_timer) == -1) {
            throw std::runtime_error { strerror(errno) };
        }
    }

   private:
    timer_t m_timer {};
};

struct timer_data {
    timer timer {};
    std::string user {};
};

} // namespace server::timing
