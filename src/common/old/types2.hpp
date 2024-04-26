#pragma once

// std
#include <cstdint>
#include <deque>

// threading
#include <threading.hpp>

namespace common {

enum class option_e : uint8_t {
    ALL,
    MINE,
    NONE,
    LINE,
    RECTANGLE,
    CIRCLE,
    TEXT
};

template <typename T>
class ts_queue {
   public:
    ts_queue() = default;

    ts_queue(const ts_queue&) = delete;

    ts_queue& operator=(const ts_queue&) = delete;

    void push_back(const T& value)
    {
        {
            threading::mutex_guard lock { m_mutex };
            m_deque.push_back(value);
        }
        m_cond_var.notify_one();
    }

    void push_back(T&& value)
    {
        {
            threading::mutex_guard lock { m_mutex };
            m_deque.push_back(value);
        }
        m_cond_var.notify_one();
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        {
            threading::mutex_guard lock { m_mutex };
            m_deque.emplace_back(std::forward<Args>(args)...);
        }
        m_cond_var.notify_one();
    }

    T pop_back()
    {
        threading::unique_mutex_guard lock { m_mutex };
        m_cond_var.wait(lock, [this]() {
            return !this->m_deque.empty();
        });
        T value = m_deque.back();
        m_deque.pop_back();
        return value;
    }

    void push_front(const T& value)
    {
        {
            threading::mutex_guard lock { m_mutex };
            m_deque.push_front(value);
        }
        m_cond_var.notify_one();
    }

    void push_front(T&& value)
    {
        {
            threading::mutex_guard lock { m_mutex };
            m_deque.push_front(value);
        }
        m_cond_var.notify_one();
    }

    template <typename... Args>
    void emplace_front(Args&&... args)
    {
        {
            threading::mutex_guard lock { m_mutex };
            m_deque.emplace_front(std::forward<Args>(args)...);
        }
        m_cond_var.notify_one();
    }

    T pop_front()
    {
        threading::unique_mutex_guard lock { m_mutex };
        m_cond_var.wait(lock, [this]() {
            return !this->m_deque.empty();
        });
        T value = m_deque.front();
        m_deque.pop_front();
        return value;
    }

   private:
    std::deque<T> m_deque {};
    threading::mutex m_mutex {};
    threading::cond_var m_cond_var {};
};

template <typename T>
class mutable_t;

template <typename T>
class readonly_t {
   public:
    explicit readonly_t(T value)
        : m_value(value)
    {
    }

    const T& operator()() const
    {
        return m_value;
    }

    friend class mutable_t<T>;

   private:
    T m_value;
};

// we use this when we want to forcefully
// modify a readonly_t
template <typename T>
class mutable_t {
   public:
    explicit mutable_t(readonly_t<T>& readonly)
        : m_readonly(readonly)
    {
    }

    T& operator()()
    {
        return m_readonly.m_value;
    }

   private:
    readonly_t<T>& m_readonly;
};

// class double_buffered_vector_t {
// public:
//    void write(size_t idx, float value) {
//       std::lock_guard<std::mutex> lk(mutWrite);
//       (*next)[idx] = value; // write to next buffer
//    }
//
//    float read(size_t idx) const {
//       std::lock_guard<std::mutex> lk(mutRead);
//       return (*current)[idx]; // read from current buffer
//    }
//
//    void swap() noexcept {
//       // Lock both mutexes safely using a deadlock
//       avoidance algorithm std::lock(mutWrite, mutRead);
//       std::lock_guard<std::mutex> lkWrite(mutWrite,
//       std::adopt_lock); std::lock_guard<std::mutex>
//       lkRead(mutRead, std::adopt_lock);
//
//       // In C++17, you can replace the 3 lines above with
//       just the following:
//       // std::scoped_lock lk( mutWrite, mutRead );
//
//       std::swap(current, next); // swap pointers
//    }
//
//    Buffer() : buf0(32), buf1(32), current(&buf0),
//    next(&buf1) { }
// private:
//    std::vector<float> buf0, buf1; // two buffers
//    std::vector<float> *current, *next;
//    mutable std::mutex mutRead;
//    std::mutex mutWrite;
//
// };

} // namespace common
