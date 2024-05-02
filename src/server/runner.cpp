// server
#include "runner.hpp"
#include "server.hpp"
#include "share.hpp"
#include "updater.hpp"

// common
#include "../common/threading.hpp"

#ifdef NETSKETCH_DUMPHASH

#include "../common/tagged_draw_vector_wrapper.hpp"

#endif

// bench
#include "../bench/bench.hpp"

// unix
#include <poll.h>
#include <unistd.h>

// cstd
#include <csignal>
#include <cstdlib>

// spdlog
#include <spdlog/async.h>
#include <spdlog/fmt/chrono.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#ifdef NETSKETCH_DUMPJSON

// cereal
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>

#endif

namespace server {

static void sigint_handler(int)
{
    // HACK or BAD: according to the standard
    // mutexes which in out case make use of
    // pthread_mutex_lock and _unlock are not
    // signal safe. Woops (seems to be working though)
    threading::mutex_guard guard { share::threads_mutex };

    for (auto& thread : share::threads) {
        thread.cancel();
    }

    share::updater_thread.cancel();
}

bool Runner::setup(uint16_t port, float time_out)
{
    // set timeout
    server::share::time_out = time_out;

    // setup signal handler

    // NOTE: using sigaction because the man page for signal says so
    struct sigaction act { };

    bzero(&act, sizeof(act));

    act.sa_handler = sigint_handler;

    if (sigaction(SIGINT, &act, nullptr) == -1) {
        fmt::println(
            stderr,
            "error: failed to create sigint handler, reason {}",
            strerror(errno)
        );

        return false;
    }

    // setup logger

    try {
        auto logger = spdlog::stdout_color_mt("server");

        spdlog::set_default_logger(logger);

        spdlog::set_level(spdlog::level::debug);
    } catch (const spdlog::spdlog_ex& ex) {
        fmt::println(stderr, "error: log init failed, reason {}", ex.what());

        return false;
    }

    m_port = port;

    return true;
}

[[nodiscard]] bool Runner::run() const
{
    START_BENCHMARK_THREAD;

    share::updater_thread = threading::thread { Updater {} };

    Server server { m_port };

    server();

    return EXIT_SUCCESS;
}

Runner::~Runner()
{
    if (share::updater_thread.is_initialized())
        share::updater_thread.join();

    {
        threading::mutex_guard guard { share::timers_mutex };

        // NOTE: https://man.archlinux.org/man/timer_create.3p.en
        // Threads allocated to timers cannot be reclaimed as
        // described in the above man package
        share::timers.clear();
    }

    END_BENCHMARK_THREAD;

#ifdef NETSKETCH_DUMPHASH
    spdlog::debug(
        "hash of tagged draw vector: {}, size of tagged draw vector {}",
        TaggedDrawVectorWrapper { share::tagged_draw_vector }.hash(),
        share::tagged_draw_vector.size()
    );
#endif

#ifdef NETSKETCH_DUMPJSON
    std::ofstream of { fmt::format("tagged_vector_server.json") };

    {
        cereal::JSONOutputArchive ar { of };

        ar(share::tagged_draw_vector);
    }
#endif
}

} // namespace client
