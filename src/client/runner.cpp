#include "runner.hpp"

// client
#include "gui.hpp"
#include "input_handler.hpp"
#include "reader.hpp"
#include "share.hpp"
#include "writer.hpp"

// common
#include "../common/overload.hpp"
#include "../common/threading.hpp"

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
#include <spdlog/common.h>
#include <spdlog/fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

// bench
#include "../bench/bench.hpp"

namespace client {

bool Runner::setup(
    bool use_gui,
    const std::string& username,
    const std::string& ipv4_addr,
    uint16_t port
)
{
    m_use_gui = use_gui;

    share::username = username;

    // setup network info

    struct in_addr addr { };

    if (inet_pton(AF_INET, ipv4_addr.c_str(), &addr) <= 0) {
        fmt::println(stderr, "error: invalid IPv4 address");

        return false;
    }

    // NOTE: this is in host-readable form
    m_ipv4_addr = ntohl(addr.s_addr);

    m_port = port;

    // setup logging

    auto now = std::chrono::system_clock::now();

    try {
        auto logger = spdlog::basic_logger_mt(
            "client",
            fmt::format("netsketch-client-log {:%Y-%m-%d %H:%M:%S}", now)
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

    fmt::println(
        "========================================================\n"
        "Connected to NetSketch server at {}:{}\n"
        "========================================================\n"
        "\n"
        "Username {} is now active. Ready to draw.\n"
        "For command list, type 'help'.",
        ipv4_addr,
        port,
        username
    );

    return true;
}

[[nodiscard]] bool Runner::run() const
{
    START_BENCHMARK_THREAD;

    share::reader_thread = threading::thread { Reader { m_channel } };
    share::writer_thread = threading::thread { Writer { m_channel } };
    share::input_thread = threading::thread { InputHandler {} };

    if (m_use_gui) {
        client::Gui gui {};

        gui();
    }

    return true;
}

Runner::~Runner()
{
    // if m_use_gui is false we will block here

    if (share::reader_thread.is_initialized())
        share::reader_thread.join();
    if (share::writer_thread.is_initialized())
        share::writer_thread.join();
    if (share::input_thread.is_initialized())
        share::input_thread.join();

    END_BENCHMARK_THREAD;
}

} // namespace client
