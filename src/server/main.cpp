// server
#include <server.hpp>

// common
#include <log.hpp>

// unix
#include <poll.h>
#include <unistd.h>

// std
#include <csignal>

// share
#include <share.hpp>

// cli11
#include <CLI/CLI.hpp>

#define SERVER_PORT (6666)

void sigint_handler(int)
{
    pollfd poll_fd {
        server::share::e_stop_event->write_fd(), POLLOUT, 0
    };

    if (poll(&poll_fd, 1, -1) == -1) {
        AbortV(
            "poll of event file descriptor failed, reason: "
            "{}",
            strerror(errno));
    }

    if (!(poll_fd.revents & POLLOUT)) {
        Abort("cannot write to event file descriptor");
    }

    uint64_t inc = 1;

    if (write(server::share::e_stop_event->write_fd(), &inc,
            sizeof(uint64_t))
        == -1) {
        AbortV("failed to write to event file descriptor, "
               "reason: {}",
            strerror(errno));
    }
}

int main(int argc, char** argv)
{
    CLI::App app;

    uint16_t port { SERVER_PORT };
    app.add_option("--port", port,
           "The port number of the NetSketch server")
        ->capture_default_str();

    CLI11_PARSE(app, argc, argv);

    common::event_t stop_event {};

    server::share::e_stop_event = &stop_event;

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

    server::server_t server { port };

    int status = server();

    return status;
}
