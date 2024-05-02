// client
#include "network_manager.hpp"
#include "share.hpp"
#include "simulate_user.hpp"

// common
#include "../common/tagged_draw_vector_wrapper.hpp"

// std
#include <fstream>
#include <regex>

// cstd
#include <cstdint>
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
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// bench
#include "../bench/bench.hpp"

// cereal
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>

void output()
{
#ifdef NETSKETCH_DUMPJSON
    std::ofstream of {
        fmt::format("tagged_vector_{}.json", test_client::share::username)
    };

    {
        cereal::JSONOutputArchive ar { of };

        ar(test_client::share::tagged_draw_vector);
    }
#endif
}

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

    std::string nickname {};
    app.add_option(
           "--nickname",
           nickname,
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

    // set the nickname of the user
    test_client::share::username = nickname;

    // set the number of expected responses
    test_client::share::expected_responses = expected_responses;

    // set the other actions flag
    test_client::share::only_drawing = !other_actions;

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

    DISABLE_INDIVIDUAL_LOGS;

    START_BENCHMARK_THREAD;

    {
        BENCH("full test client run");

        test_client::NetworkManager manager { ipv4_addr, port };

        if (!manager.setup()) {
            fmt::println("error: failed to connect to server");

            END_BENCHMARK_THREAD;

            return EXIT_FAILURE;
        }

        test_client::simulate_behaviour(iterations, interval);
    }

    spdlog::info("Finished...");

    END_BENCHMARK_THREAD;

#ifdef NETSKETCH_DUMPHASH
    spdlog::debug(
        "hash of tagged draw vector: {}, size of tagged draw vector {}",
        TaggedDrawVectorWrapper { test_client::share::tagged_draw_vector }.hash(
        ),
        test_client::share::tagged_draw_vector.size()

    );
#endif

    output();

    return 0;
}
