// server
#include "server.hpp"
#include "share.hpp"
#include "updater.hpp"

// common
#include "../common/abort.hpp"
#include "../common/threading.hpp"

// unix
#include <poll.h>
#include <unistd.h>

// std
#include <csignal>

// cli11
#include <CLI/CLI.hpp>

#define SERVER_PORT (6666)

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

    uint16_t port { SERVER_PORT };
    app.add_option("--port", port, "The port number of the NetSketch server")
        ->capture_default_str();

    CLI11_PARSE(app, argc, argv);

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

    server::share::updater_thread = threading::thread { server::Updater {} };

    server::Server server { port };

    server();

    server::share::updater_thread.join();

    {
        threading::mutex_guard guard { server::share::timers_mutex };

        server::share::timers.clear();
    }

    return 0;
}
