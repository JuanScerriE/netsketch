// client
#include "reader.hpp"
#include "share.hpp"

// common
#include "../common/overload.hpp"
#include "../common/tagged_draw_vector_wrapper.hpp"
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
        if (share::tagged_draw_vector.size() >= share::expected_responses)
            break;

        std::pair<ByteString, ChannelError> res {};

        {
            BENCH("reading input from network");
            // NOTE: that the m_channel.read() blocks
            res = m_channel.read();
        }

        auto [bytes, status] = res;

        if (status == ChannelErrorCode::DESERIALIZATION_FAILED) {
            spdlog::error("deserialization failed, reason {}", status.what());

            // spdlog::debug("number of received responses: {}", m_responses);

            break;
        }

        if (status != ChannelErrorCode::OK) {
            spdlog::error("reading failed, reason {}", status.what());

            break;
        }

        handle_payload(bytes);
    }

    shutdown();
}

void Reader::handle_payload(ByteString& bytes)
{
    BENCH("handling payload");

    auto [payload, status] = deserialize<Payload>(bytes);

    if (status != DeserializeErrorCode::OK) {
        spdlog::warn("deserialization failed, reason {}", status.what());

        return;
    }

    std::visit(
        overload {
            [](Adopt& arg) {
                TaggedDrawVectorWrapper { share::tagged_draw_vector }.adopt(arg
                );
            },
            [](TaggedDrawVector& arg) {
                share::tagged_draw_vector = arg;
            },
            [](TaggedAction& arg) {
                TaggedDrawVectorWrapper { share::tagged_draw_vector }.update(arg
                );
            },
            [](auto& object) {
                spdlog::warn(
                    "unexpected payload type {}",
                    typeid(object).name()
                );
            },
        },
        payload
    );
}

void Reader::shutdown()
{
    if (share::writer_thread.is_initialized()
        && share::writer_thread.is_alive())
        share::writer_thread.cancel();
}

} // namespace test_client
