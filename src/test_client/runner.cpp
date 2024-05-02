
// test_client
#include "runner.hpp"
#include "reader.hpp"
#include "share.hpp"
#include "simulate_user.hpp"
#include "writer.hpp"

// common
#include "../common/overload.hpp"
#include "../common/tagged_draw_vector_wrapper.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// cstd
#include <cstdlib>

// unix
#include <arpa/inet.h>
#include <netinet/in.h>

// fmt
#include <fmt/chrono.h>
#include <fmt/format.h>

// spdlog
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// bench
#include "../bench/bench.hpp"

#ifdef NETSKETCH_DUMPJSON

// std
#include <fstream>

// cereal
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>

#endif

namespace test_client {

bool Runner::setup(
    const std::string& ipv4_addr,
    uint16_t port,
    uint32_t iterations,
    double interval,
    uint32_t expected_responses,
    const std::string& username,
    bool other_actions
)
{
    // setup network info

    struct in_addr addr { };

    if (inet_pton(AF_INET, ipv4_addr.c_str(), &addr) <= 0) {
        fmt::println(stderr, "error: invalid IPv4 address");

        return false;
    }

    // NOTE: this is in host-readable form
    m_ipv4_addr = ntohl(addr.s_addr);

    m_port = port;

    m_iterations = iterations;

    m_interval = interval;

    share::expected_responses = expected_responses;

    share::only_drawing = !other_actions;

    share::username = username;

    // setup logging

    try {
        auto logger
            = spdlog::stdout_color_mt(fmt::format("test_client ({})", username)
            );

        spdlog::set_default_logger(logger);

        spdlog::set_level(spdlog::level::debug);
    } catch (const spdlog::spdlog_ex& ex) {
        fmt::println(stderr, "error: log init failed, reason {}", ex.what());

        return false;
    }

    // setup socket

    try {
        m_sock.open(SOCK_STREAM, 0);

        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(m_ipv4_addr);
        addr.sin_port = htons(m_port);

        m_sock.connect(&addr);
    } catch (std::runtime_error& error) {
        fmt::println(
            stderr,
            "error: opening & connecting failed, reason {}",
            error.what()
        );

        return false;
    }

    m_channel = Channel { m_sock };

    // check username

    ByteString req { serialize<Payload>(Username { username }) };

    auto write_status = m_channel.write(req);

    if (write_status != ChannelErrorCode::OK) {
        fmt::println(
            stderr,
            "error: writing failed, reason {}",
            write_status.what()
        );

        return false;
    }

    auto [res, read_status] = m_channel.read(60000); // wait for a minute

    if (read_status != ChannelErrorCode::OK) {
        fmt::println(
            stderr,
            "error: reading failed, reason {}",
            read_status.what()
        );

        return false;
    }

    auto [payload, status] = deserialize<Payload>(res);

    if (status != DeserializeErrorCode::OK) {
        fmt::println(
            stderr,
            "error: deserialization failed, reason {}",
            status.what()
        );

        return false;
    }

    if (std::holds_alternative<Decline>(payload)) {
        fmt::println(
            stderr,
            "error: connection declined, reason {}",
            std::get<Decline>(payload).reason
        );

        return false;
    }

    if (!std::holds_alternative<Accept>(payload)) {
        fmt::println(
            stderr,
            "error: unexpected payload type {}",
            var_type(payload).name()
        );

        return false;
    }

    return true;
}

[[nodiscard]] bool Runner::run() const
{
    DISABLE_INDIVIDUAL_LOGS;

    START_BENCHMARK_THREAD;

    share::reader_thread = threading::thread { Reader { m_channel } };
    share::writer_thread = threading::thread { Writer { m_channel } };

    simulate_behaviour(m_iterations, m_interval);

    return EXIT_SUCCESS;
}

Runner::~Runner()
{
    if (share::reader_thread.is_initialized())
        share::reader_thread.join();
    if (share::writer_thread.is_initialized())
        share::writer_thread.join();

    END_BENCHMARK_THREAD;

#ifdef NETSKETCH_DUMPHASH
    spdlog::debug(
        "hash of tagged draw vector: {}, size of tagged draw vector {}",
        TaggedDrawVectorWrapper { share::tagged_draw_vector }.hash(),
        share::tagged_draw_vector.size()
    );
#endif

#ifdef NETSKETCH_DUMPJSON
    std::ofstream of { fmt::format("tagged_vector_server.json") };

    {
        cereal::JSONOutputArchive ar { of };

        ar(share::tagged_draw_vector);
    }
#endif

    spdlog::info("Finished...");
}

} // namespace test_client
