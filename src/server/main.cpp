// server
#include "threading.hpp"
#include <server.hpp>
#include <updater.hpp>

// common
#include <log.hpp>

// unix
#include <poll.h>
#include <unistd.h>

// std
#include <csignal>
#include <thread>

// share
#include <share.hpp>

// cli11
#include <CLI/CLI.hpp>

#define SERVER_PORT (6666)

void sigint_handler(int)
{
    threading::lock_guard guard {
        server::share::e_threads_mutex
    };

    for (auto& thread : server::share::e_threads) {
        thread.cancel();
    }

    server::share::e_updater_thread.cancel();

    server::share::e_server_thread.cancel();
}

int main(int argc, char** argv)
{
    CLI::App app;

    uint16_t port { SERVER_PORT };
    app.add_option("--port", port,
           "The port number of the NetSketch server")
        ->capture_default_str();

    CLI11_PARSE(app, argc, argv);

    // NOTE: are using sigaction because the man page for
    // signal says so
    struct sigaction act { };

    bzero(&act, sizeof(act));

    act.sa_handler = sigint_handler;

    if (sigaction(SIGINT, &act, nullptr) == -1) {
        AbortV("failed to create sigint handler, "
               "reason: {}",
            strerror(errno));
    }

    server::share::e_updater_thread
        = threading::pthread { server::updater_t {} };

    // NOTE: we are indeed wasting a bit of
    // resources associated with the main thread,
    // however, the wrapping & isolation provided
    // by threading::pthread is too convenient to
    // pass up on.
    server::share::e_server_thread
        = threading::pthread { server::server_t { port } };

    server::share::e_server_thread.join();

    server::share::e_updater_thread.join();

    return 0;
}
