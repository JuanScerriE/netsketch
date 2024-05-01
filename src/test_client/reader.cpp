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
    for (;;) {
        if (m_responses >= share::expected_responses)
            break;

        BENCH("reading network input");

        auto [bytes, status] = m_channel.read();

        if (status == ChannelErrorCode::DESERIALIZATION_FAILED) {
            spdlog::error("deserialization failed, reason {}", status.what());

            spdlog::debug(
                "actual number of responses received: {}, with hash {}",
                m_responses,
                m_hash
            );

            // NOTE: we want to manually induce a coredump
            std::raise(SIGABRT);
        }

        if (status != ChannelErrorCode::OK) {
            spdlog::error("reading failed, reason {}", status.what());

            break;
        }

        m_hash = m_hash ^ (std::hash<ByteString> {}(bytes) << 1);

        m_responses++;
    }

    shutdown();
}

void Reader::shutdown()
{
    spdlog::debug(
        "actual number of responses received: {}, with hash {}",
        m_responses,
        m_hash
    );

    if (share::writer_thread.is_initialized()
        && share::writer_thread.is_alive())
        share::writer_thread.cancel();
}

} // namespace test_client
