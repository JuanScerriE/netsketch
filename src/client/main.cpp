// client
#include <cstdlib>
#include <gui.hpp>
#include <input_handler.hpp>
#include <log_file.hpp>
#include <network_manager.hpp>

// common
#include <types.hpp>

// threading
#include <threading.hpp>

// std
#include <regex>

// unix
#include <arpa/inet.h>

// cli11
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>

// fmt
#include <fmt/chrono.h>
#include <fmt/format.h>

// share
#include <share.hpp>

struct IPv4Validator : public CLI::Validator {
    IPv4Validator()
    {
        name_ = "IPv4";
        func_ = [](const std::string& str) {
            std::regex ipv4_regex {
                R"(^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}(25[0-5]|(2[0-4]|1\d|[1-9]|)\d)$)"
            };

            if (!std::regex_match(str, ipv4_regex))
                return std::string {
                    "string is not a IPv4 address"
                };
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
    app.add_option("--server", ipv4_addr_str,
           "The IPv4 address of a machine hosting a "
           "NetSketch server")
        ->capture_default_str()
        ->check(IPv4);

    uint16_t port { 6666 };
    app.add_option("--port", port,
           "The port number of a NetSketch server")
        ->capture_default_str();

    std::string nickname {};
    app.add_option("--nickname", nickname,
           "The nickname of the user (used for "
           "identification on a NetSketch server)")
        ->required();

    CLI11_PARSE(app, argc, argv);

    // set the nickname of the user
    client::share::nickname = nickname;

    in_addr addr {};

    if (inet_pton(AF_INET, ipv4_addr_str.c_str(), &addr)
        <= 0) {
        // NOTE: not using AbortIf since the above is
        // actually causing a mutation, so I just
        // want that to be clear
        Abort("invalid IPv4 address");
    }

    // NOTE: this is in host-readable form
    uint32_t ipv4_addr = ntohl(addr.s_addr);

    // create a log file
    using std::chrono::system_clock;

    auto now = system_clock::now();

    client::share::log_file.open(fmt::format(
        "netsketch-client-log {:%Y-%m-%d %H:%M:%S}", now));

    AbortIfV(client::share::log_file.error(),
        "opening a log file failed, reason: {}",
        client::share::log_file.reason());

    client::network_manager_t manager { ipv4_addr, port };

    if (!manager.setup()) {
        if (client::share::log_file.is_open()) {
            client::share::log_file.close();
        }

        fmt::println("error: failed to connect to server");

        return EXIT_FAILURE;
    }

    client::share::input_thread
        = threading::pthread { client::input_handler_t {} };

    client::gui_t gui {};

    // NOTE: we are running the GUI in the main
    // thread because of macOS, that is macOS
    // does not like GLFW which raylib uses
    // running in a separate thread
    gui();

    client::share::input_thread.join();

    manager.close();

    if (client::share::log_file.is_open()) {
        client::share::log_file.close();
    }

    return 0;
}
