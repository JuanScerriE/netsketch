// client
#include "reader.hpp"
#include "share.hpp"

// common
#include "../common/threading.hpp"

// fmt
#include <fmt/chrono.h>
#include <fmt/core.h>

// unix
#include <netinet/in.h>
#include <poll.h>
#include <sys/poll.h>
#include <unistd.h>

// cstd
#include <csignal>

// spdlog
#include <spdlog/spdlog.h>

// bench
#include "../bench/bench.hpp"

namespace test_client {

Reader::Reader(const Channel& channel)
    : m_channel(channel)
{
}

void Reader::operator()()
{
    read_loop();
}

void Reader::read_loop()
{
    for (;;) {
        BENCH("reading network input");

        auto [_, status] = m_channel.read();

        if (status == ChannelErrorCode::DESERIALIZATION_FAILED) {
            spdlog::error("deserialization failed, reason {}", status.what());

            // NOTE: we want to manually induce a coredump
            std::raise(SIGABRT);
        }

        if (status != ChannelErrorCode::OK) {
            spdlog::error("reading failed, reason {}", status.what());

            break;
        }
    }

    shutdown();
}

void Reader::shutdown()
{
    if (share::writer_thread.is_initialized()
        && share::writer_thread.is_alive())
        share::writer_thread.cancel();
}

} // namespace test_client
