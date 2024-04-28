#pragma once

// std
#include <chrono>

// spdlog
#include <spdlog/spdlog.h>

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
            = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_start)
                  .time_since_epoch();
        auto end = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_end)
                       .time_since_epoch();

        auto time_taken = end - start;

        spdlog::debug(
            "[{}:{}:{}] \"{}\" Time Taken: {}",
            m_file,
            m_function_name,
            m_line,
            m_name,
            time_taken
        );
    }

   private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start {};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_end {};

    std::string m_file {};
    std::string m_function_name {};
    std::string m_line {};
    std::string m_name {};
};

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

#ifdef BENCHMARK
#define BENCH(name) \
    (Bench { file_name(CompTimeString { __FILE__ }), __func__, LINE_STRING, name })
#else
#define BENCH(name)
#endif
