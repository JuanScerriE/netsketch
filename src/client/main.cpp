// client
#include "gui.hpp"
#include "input_handler.hpp"
#include "network_manager.hpp"
#include "share.hpp"

// common
#include "../common/threading.hpp"

// std
#include <regex>

// cstd
#include <cstdlib>

// unix
#include <arpa/inet.h>

// cli11
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>

// fmt
#include <fmt/chrono.h>
#include <fmt/format.h>

// spdlog
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

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

    bool use_gui { true };
    app.add_flag("--gui, !--no-gui", use_gui, "Use a GUI")
        ->capture_default_str();

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

    std::string nickname {};
    app.add_option(
           "--nickname",
           nickname,
           "The nickname of the user (used for "
           "identification on a NetSketch server)"
    )
        ->required();

    CLI11_PARSE(app, argc, argv);

    // set the nickname of the user
    client::share::username = nickname;

    in_addr addr {};

    if (inet_pton(AF_INET, ipv4_addr_str.c_str(), &addr) <= 0) {
        // NOTE: not using ABORTIF since the above is
        // actually causing a mutation, so I just
        // want that to be clear
        ABORT("invalid IPv4 address");
    }

    // NOTE: this is in host-readable form
    uint32_t ipv4_addr = ntohl(addr.s_addr);

    auto now = std::chrono::system_clock::now();

    try {
        auto logger = spdlog::basic_logger_mt(
            "client",
            fmt::format("netsketch-client-log {:%Y-%m-%d %H:%M:%S}", now)
        );

        spdlog::set_default_logger(logger);

        spdlog::set_level(spdlog::level::debug);
    } catch (const spdlog::spdlog_ex& ex) {
        fmt::println("log init failed: {}", ex.what());

        return EXIT_FAILURE;
    }

    client::NetworkManager manager { ipv4_addr, port };

    if (!manager.setup()) {
        fmt::println("error: failed to connect to server");

        return EXIT_FAILURE;
    }

    client::share::input_thread = threading::thread { client::InputHandler {} };

    client::Gui gui {};

    // NOTE: we are running the GUI in the main
    // thread because of macOS, that is macOS
    // does not like GLFW which raylib uses
    // running in a separate thread
    if (use_gui) {
        gui();
    }

    client::share::input_thread.join();

    return 0;
}
