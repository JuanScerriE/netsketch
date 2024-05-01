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
        BENCH("reading network input");

        auto [res, status] = m_channel.read();

        if (status == ChannelErrorCode::DESERIALIZATION_FAILED) {
            spdlog::error("deserialization failed, reason {}", status.what());

            break;
        }

        if (status != ChannelErrorCode::OK) {
            spdlog::error("reading failed, reason {}", status.what());

            break;
        }

        handle_payload(res);
    }

    shutdown();
}

void Reader::handle_payload(ByteString& bytes)
{
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
