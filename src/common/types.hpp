#pragma once

// std
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <variant>

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

struct colour_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

template <typename T>
class queue_st {
   public:
    queue_st() = default;

    queue_st(const queue_st&) = delete;

    queue_st& operator=(const queue_st&) = delete;

    void push_back(const T& value) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_deque.push_back(std::forward<T>(value));
        m_cond_var.notify_one();
    }

    void push_back(T&& value) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_deque.push_back(std::forward<T>(value));
        m_cond_var.notify_one();
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_deque.emplace_back(std::forward<Args>(args)...);
        m_cond_var.notify_one();
    }

    T pop_back() {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_cond_var.wait(lock, [this]() {
            return this->m_deque.empty();
        });
        T value = m_deque.back();
        m_deque.pop_back();
        return value;
    }

    void push_front(const T& value) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_deque.push_front(std::forward<T>(value));
        m_cond_var.notify_one();
    }

    void push_front(T&& value) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_deque.push_front(std::forward<T>(value));
        m_cond_var.notify_one();
    }

    template <typename... Args>
    void emplace_front(Args&&... args) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_deque.emplace_front(std::forward<Args>(args)...);
        m_cond_var.notify_one();
    }

    T pop_front() {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_cond_var.wait(lock, [this]() {
            return this->m_deque.empty();
        });
        T value = m_deque.front();
        m_deque.pop_front();
        return value;
    }

   private:
    std::deque<T> m_deque{};
    std::mutex m_mutex{};
    std::condition_variable m_cond_var{};
};

struct line_draw_t {
    colour_t colour;
    int x0;
    int y0;
    int x1;
    int y1;
};

struct rectangle_draw_t {
    colour_t colour;
    int x;
    int y;
    int w;
    int h;
};

struct circle_draw_t {
    colour_t colour;
    int x;
    int y;
    float r;
};

struct text_draw_t {
    colour_t colour;
    int x;
    int y;
    std::string string;
};

using draw_t = std::variant<
    line_draw_t,
    rectangle_draw_t,
    circle_draw_t,
    text_draw_t>;

template <typename T> class mutable_t;

template <typename T>
class readonly_t {
   public:
    explicit readonly_t(T value)
        : m_value(value) {
    }

    const T& operator()() const {
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
        : m_readonly(readonly) {
    }

    T& operator()() {
        return m_readonly.m_value;
    }

   private:
    readonly_t<T>& m_readonly;
};

}  // namespace common
