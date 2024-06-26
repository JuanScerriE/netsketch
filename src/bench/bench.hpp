#pragma once

// std
#include <chrono>

// spdlog
#include <spdlog/fmt/chrono.h>
#include <spdlog/spdlog.h>

// common
#include "../common/threading.hpp"

// std
#include <queue>

// The below class is small class which is starts a timer when
// its created and closes a timer when it is destroyed.
// The way in which this class is suppose
// to be used is by creating a scope and constructing the
// Bench object within that scope. This basically,
// allows us to time particular scope.

namespace bench {

extern threading::thread benchmark_thread;
extern threading::mutex benchmark_mutex;
extern threading::cond_var benchmark_cond;
extern std::queue<std::pair<std::string, std::chrono::microseconds>>
    benchmark_queue;

extern bool disable_individual_logs;

class MovingBenchmark {
   public:
    void operator()()
    {
        for (;;) {
            threading::unique_mutex_guard guard { benchmark_mutex };

            benchmark_cond.wait(guard, []() {
                return benchmark_queue.empty();
            });

            auto [key, time_taken] = benchmark_queue.front();

            benchmark_queue.pop();

            m_moving_averages[key]
                = (m_moving_averages.count(key) <= 0)
                      ? time_taken
                      : (m_moving_averages[key] + time_taken) / 2;
        }
    }

    ~MovingBenchmark()
    {
        for (auto& [key, moving_average] : m_moving_averages) {
            spdlog::debug("moving average of \"{}\": {}", key, moving_average);
        }
    }

   private:
    std::unordered_map<std::string, std::chrono::microseconds>
        m_moving_averages {};
};

class Bench {
   public:
    explicit Bench(
        const char* file,
        const char* function_name,
        const char* line,
        const char* name
    )
        : m_file { file }
        , m_function_name { function_name }
        , m_line { line }
        , m_name { name }
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    explicit Bench(
        std::string file,
        std::string function_name,
        std::string line,
        std::string name
    )
        : m_file { std::move(file) }
        , m_function_name { std::move(function_name) }
        , m_line { std::move(line) }
        , m_name { std::move(name) }
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~Bench()
    {
        m_end = std::chrono::high_resolution_clock::now();

        auto start
            = std::chrono::time_point_cast<std::chrono::microseconds>(m_start)
                  .time_since_epoch();
        auto end
            = std::chrono::time_point_cast<std::chrono::microseconds>(m_end)
                  .time_since_epoch();

        std::chrono::microseconds time_taken = (end - start);

        {
            threading::mutex_guard guard { benchmark_mutex };

            benchmark_queue.emplace(m_name, time_taken);
        }

        benchmark_cond.notify_one();

        if (!disable_individual_logs) {
            spdlog::debug(
                "[{}:{}:{}] \"{}\" Time Taken: {}",
                m_file,
                m_function_name,
                m_line,
                m_name,
                time_taken
            );
        }
    }

   private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start {};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_end {};

    std::string m_file {};
    std::string m_function_name {};
    std::string m_line {};
    std::string m_name {};
};

} // namespace bench

// We are using the notion of a compile time string
// because the compiler is capable of injecting
// information about our source into the source code
// itself with the __FILE__ and __LINE__ macros
// along with the __func__ value. The only issue
// is that __FILE__ provides an absolute path
// which I think is a bit too clunky. So, instead
// we'll use the comptime facilities provided to use
// by constexpr and the CompTimeString below
// to modify the result of __FILE__ at compile
// time into just the file name.

// ATTRIBUTION
// https://stackoverflow.com/questions/77802986/compile-time-string-manipulation

template <std::size_t N>
struct CompTimeString {
    char bytes[N];

    [[nodiscard]] constexpr std::size_t size() const
    {
        std::size_t r = 0;
        while (r + 1 < N && bytes[r])
            ++r;
        return r;
    }

    constexpr char& operator[](std::size_t i)
    {
        return bytes[i];
    }

    constexpr char const& operator[](std::size_t i) const
    {
        return bytes[i];
    }

    explicit constexpr CompTimeString(char const (&arr)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
            bytes[i] = arr[i];
    }

    explicit constexpr CompTimeString()
    {
        for (std::size_t i = 0; i < N; ++i)
            bytes[i] = '\0';
    }

    [[nodiscard]] constexpr char const* data() const
    {
        return bytes;
    }

    constexpr operator char const*() const
    {
        return data();
    }
};

template <std::size_t N>
constexpr CompTimeString<N> file_name(CompTimeString<N> file_path)
{
    CompTimeString<N> name {};

    std::size_t j = 0;

    for (std::size_t i = 0; i < N; i++) {
        if (file_path[i] == '/') {
            j = 0;
        } else {
            name[j++] = file_path[i];
        }
    }

    return name;
}

#ifndef STRINGIFY_
#define STRINGIFY_(x) #x
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) STRINGIFY_(x)
#endif

#ifndef LINE_STRING
#define LINE_STRING STRINGIFY(__LINE__)
#endif

#ifdef NETSKETCH_BENCHMARK

#define DISABLE_INDIVIDUAL_LOGS                \
    do {                                       \
        bench::disable_individual_logs = true; \
    } while (0)

#define START_BENCHMARK_THREAD                                 \
    do {                                                       \
        bench::benchmark_thread                                \
            = threading::thread { bench::MovingBenchmark {} }; \
    } while (0)

#define END_BENCHMARK_THREAD                            \
    do {                                                \
        if (bench::benchmark_thread.is_initialized()) { \
            bench::benchmark_thread.cancel();           \
            bench::benchmark_thread.join();             \
        }                                               \
    } while (0)

// This is the macro we use to instead of calling
// Bench {} directly to automatically fill out
// all the info relating to the file name,
// function name and line number.

#define BENCH(name)                                                         \
    bench::Bench netsketch_server_internal_bench                            \
    {                                                                       \
        file_name(CompTimeString { __FILE__ }), __func__, LINE_STRING, name \
    }

#else

#define DISABLE_INDIVIDUAL_LOGS
#define START_BENCHMARK_THREAD
#define END_BENCHMARK_THREAD
#define BENCH(name)

#endif
