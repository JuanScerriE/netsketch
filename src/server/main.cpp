// server
#include "server.hpp"
#include "share.hpp"
#include "updater.hpp"

// common
#include "../common/abort.hpp"
#include "../common/threading.hpp"

// bench
#include "../bench/bench.hpp"

// unix
#include <cstdlib>
#include <poll.h>
#include <unistd.h>

// cstd
#include <csignal>

// cli11
#include <CLI/CLI.hpp>

// spdlog
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/fmt/chrono.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

void sigint_handler(int)
{
    // HACK or BAD: according to the standard
    // mutexes which in out case make use of
    // pthread_mutex_lock and _unlock are not
    // signal safe. Woops (seems to working
    // though)
    threading::mutex_guard guard { server::share::threads_mutex };

    for (auto& thread : server::share::threads) {
        thread.cancel();
    }

    server::share::updater_thread.cancel();
}

int main(int argc, char** argv)
{
    CLI::App app;

    uint16_t port { 6666 };
    app.add_option("--port", port, "The port number of the NetSketch server")
        ->capture_default_str();

    float time_out { 10 };
    app.add_option(
           "--time-out",
           time_out,
           "The time out (in minutes) for an inactive client"
    )
        ->capture_default_str();

    CLI11_PARSE(app, argc, argv);

    // set timeout
    server::share::time_out = time_out;

    // NOTE: are using sigaction because the man page for
    // signal says so
    struct sigaction act { };

    bzero(&act, sizeof(act));

    act.sa_handler = sigint_handler;

    if (sigaction(SIGINT, &act, nullptr) == -1) {
        ABORTV(
            "failed to create sigint handler, "
            "reason: {}",
            strerror(errno)
        );
    }

    try {
        auto logger = spdlog::stdout_color_mt("server");

        spdlog::set_default_logger(logger);

        spdlog::set_level(spdlog::level::debug);
    } catch (const spdlog::spdlog_ex& ex) {
        fmt::println("log init failed: {}", ex.what());

        return EXIT_FAILURE;
    }

    START_BENCHMARK_THREAD;

    server::share::updater_thread = threading::thread { server::Updater {} };

    server::Server server { port };

    server();

    server::share::updater_thread.join();

    {
        threading::mutex_guard guard { server::share::timers_mutex };

        // NOTE: https://man.archlinux.org/man/timer_create.3p.en
        // Threads allocated to timers cannot be reclaimed as
        // described in the above man package
        server::share::timers.clear();
    }

    END_BENCHMARK_THREAD;

    return 0;
}
