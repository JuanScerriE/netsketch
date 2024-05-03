// client
#include "runner.hpp"

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

    std::string username {};
    app.add_option(
           "--username",
           username,
           "The nickname of the user (used for "
           "identification on a NetSketch server)"
    )
        ->required();

    std::string ipv4_addr { "127.0.0.1" };
    app.add_option(
           "--ipv4",
           ipv4_addr,
           "IPv4 address of machine hosting a server"
    )
        ->capture_default_str()
        ->check(IPv4);

    uint16_t port { 6666 };
    app.add_option("--port", port, "port number of a NetSketch server")
        ->capture_default_str();

    CLI11_PARSE(app, argc, argv);

    exporter::Runner runner {};

    if (!runner.setup(username, ipv4_addr, port)) {
        return EXIT_FAILURE;
    }

    if (!runner.run()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
