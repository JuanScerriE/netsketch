// server
#include "runner.hpp"

// cli11
#include <CLI/CLI.hpp>

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

    server::Runner runner {};

    if (!runner.setup(port, time_out)) {
        return EXIT_FAILURE;
    }

    if (!runner.run()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
