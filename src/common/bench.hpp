#pragma once

// std
#include <chrono>

// spdlog
#include <spdlog/spdlog.h>

// The below class is small class which is starts a timer when
// its created and closes a timer when it is destroyed.
// The way in which this class is suppose
// to be used is by creating a scope and constructing the
// Bench object within that scope. This basically,
// allows us to time particular scope.

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

#ifdef BENCHMARK

// This is the macro we use to instead of calling
// Bench{} directly to automatically fill out
// all the info relating to the file name,
// function name and line number.

#define BENCH(name)                                  \
    (Bench { file_name(CompTimeString { __FILE__ }), \
             __func__,                               \
             LINE_STRING,                            \
             name })
#else
#define BENCH(name)
#endif
