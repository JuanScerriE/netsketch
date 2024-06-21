// client_test
#include "runner.hpp"

// bench
#include "../bench/bench.hpp"

// std
#include <regex>

// cli11
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>

struct IPv4Validator : public CLI::Validator {
    IPv4Validator()
    {
        name_ = "IPv4";
        func_ = [](const std::string& str) {
            std::regex ipv4_regex {
                R"(^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}(25[0-5]|(2[0-4]|1\d|[1-9]|)\d)$)"
            };

            if (!std::regex_match(str, ipv4_regex))
                return std::string { "string is not a IPv4 address" };
            else
                return std::string {};
        };
    }
};

const static IPv4Validator IPv4;

int main(int argc, char** argv)
{
    CLI::App app;

    std::string ipv4_addr_str { "127.0.0.1" };
    app.add_option(
           "--server",
           ipv4_addr_str,
           "The IPv4 address of a machine hosting a "
           "NetSketch server"
    )
        ->capture_default_str()
        ->check(IPv4);

    uint16_t port { 6666 };
    app.add_option("--port", port, "The port number of a NetSketch server")
        ->capture_default_str();

    uint32_t iterations { 100 };
    app.add_option(
           "--iterations",
           iterations,
           "The number of requests the Test Client will send to the NetSketch "
           "server"
    )
        ->capture_default_str();

    double interval { 1.0 };
    app.add_option(
           "--interval",
           interval,
           "The interval between number of requests the Test Client will send "
           "to the NetSketch server"
    )
        ->capture_default_str();

    uint32_t expected_responses { 0 };
    app.add_option(
           "--expected-responses",
           expected_responses,
           "The number of expected responses from the NetSketch Server"
    )
        ->required();

    std::string username {};
    app.add_option(
           "--username",
           username,
           "The nickname of the user (used for "
           "identification on a NetSketch server)"
    )
        ->required();

    bool other_actions { false };
    app.add_flag(
           "--other-actions, !--other-actions",
           other_actions,
           "Enable generating other actions apart from drawing (e.g. Undo, "
           "Clear & Delete)"
    )
        ->capture_default_str();

    CLI11_PARSE(app, argc, argv);

    test_client::Runner runner {};

    if (!runner.setup(
            ipv4_addr_str,
            port,
            iterations,
            interval,
            expected_responses,
            username,
            other_actions
        )) {
        return EXIT_FAILURE;
    }

    {
        BENCH("full test client run");

        if (!runner.run()) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
