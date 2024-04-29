// client
#include "network_manager.hpp"
#include "share.hpp"

// std
#include <regex>

// cstd
#include <cstdlib>
#include <ctime>

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
#include <spdlog/sinks/stdout_color_sinks.h>
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

void simulate_behaviour();

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

    try {
        auto logger
            = spdlog::stdout_color_mt(fmt::format("test_client ({})", nickname)
            );

        spdlog::set_default_logger(logger);

        spdlog::set_level(spdlog::level::debug);
    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "log init failed: " << ex.what() << std::endl;

        return 0;
    }

    client::NetworkManager manager { ipv4_addr, port };

    if (!manager.setup()) {
        fmt::println("error: failed to connect to server");

        return EXIT_FAILURE;
    }

    simulate_behaviour();

    spdlog::info("Finished...");

    return 0;
}

const std::vector<Action> set_of_actions = {
    LineDraw { Colour { 255, 34, 0 }, 20, 110, 400, 100 },
    RectangleDraw { Colour { 255, 0, 255 }, 500, 500, 300, 400 },
    CircleDraw { Colour { 255, 30, 255 }, 540, 000, 25.345f },
    TextDraw { Colour { 255, 0, 0 },
               -100,
               -200,
               "THIS IS A "
               "VEERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
               "RRRRRRRRRRRRY LONG" },
    Undo {},
    Clear { Qualifier::MINE },
    Clear { Qualifier::ALL },
    Delete { 0 },
    Delete { 23 },
};

const int time_delta = 1;

void simulate_behaviour()
{
    int max_iterations = 100;

    srand(static_cast<uint32_t>(time(nullptr)));

    for (int i = 0; i < max_iterations; i++) {
        sleep(time_delta);

        int action = rand() % static_cast<int>(set_of_actions.size());

        {
            threading::mutex_guard guard { client::share::writer_mutex };

            client::share::writer_queue.push(
                set_of_actions[static_cast<size_t>(action)]
            );
        }

        client::share::writer_cond.notify_one();
    }
}
