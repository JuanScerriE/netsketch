// client
#include "reader.hpp"
#include "share.hpp"

// common
#include "../common/serial.hpp"
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
#include <variant>

// spdlog
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

// bench
#include "../bench/bench.hpp"

namespace client {

Reader::Reader(const Channel& channel)
    : m_channel(channel)
{
}

void Reader::operator()()
{
    for (;;) {
        std::pair<ByteString, ChannelError> res {};

        {
            BENCH("reading input from network");
            // NOTE: that the m_channel.read() blocks
            res = m_channel.read();
        }

        auto [bytes, status] = res;

        if (status != ChannelErrorCode::OK) {
            // HACK: to solve the problem of printing in an concurrent
            // application we'd need to use linenoise with its async mode
            fmt::println(
                stderr,
                "\nerror: reading failed, reason {}",
                status.what()
            );

            break;
        }

        if (!handle_payload(bytes))
            break;
    }

    shutdown();
}

bool Reader::handle_payload(ByteString& bytes)
{
    BENCH("handling payload");

    auto [payload, status] = deserialize<Payload>(bytes);

    if (status != DeserializeErrorCode::OK) {
        fmt::println(
            stderr,
            "\nerror: deserialization failed, reason {}",
            status.what()
        );

        return false;
    }

    // NOTE: again please go look at client/share.hpp
    // for a reason as to why we are updating two separate
    // objects which contain the same things using a rwlock
    return std::visit(
        overload {
            [](Adopt& arg) {
                threading::mutex_guard guard {
                    share::tagged_draw_vector_mutex
                };

                {
                    threading::rwlock_wrguard wrguard { share::rwlock1 };

                    TaggedDrawVectorWrapper { share::vec1 }.adopt(arg);
                }

                {
                    threading::rwlock_wrguard wrguard { share::rwlock2 };

                    TaggedDrawVectorWrapper { share::vec2 }.adopt(arg);
                }

                return true;
            },
            [](TaggedDrawVector& arg) {
                threading::mutex_guard guard {
                    share::tagged_draw_vector_mutex
                };

                {
                    threading::rwlock_wrguard wrguard { share::rwlock1 };

                    share::vec1 = arg;
                }

                {
                    threading::rwlock_wrguard wrguard { share::rwlock2 };

                    share::vec2 = arg;
                }

                return true;
            },
            [](TaggedAction& arg) {
                threading::mutex_guard guard {
                    share::tagged_draw_vector_mutex
                };

                {
                    threading::rwlock_wrguard wrguard { share::rwlock1 };

                    TaggedDrawVectorWrapper { share::vec1 }.update(arg);
                }

                {
                    threading::rwlock_wrguard wrguard { share::rwlock2 };

                    TaggedDrawVectorWrapper { share::vec2 }.update(arg);
                }

                return true;
            },
            [](auto& object) {
                fmt::println(
                    stderr,
                    "\nerror: unexpected payload type {}",
                    typeid(object).name()
                );

                return false;
            },
        },
        payload
    );
}

void Reader::shutdown()
{
    // The confiugration and setup is limited enough
    // to allow us to stop things manually from each
    // of the components. In particular the same
    // few steps are being done in the writer and
    // the input handler.
    if (share::writer_thread.is_initialized()
        && share::writer_thread.is_alive())
        share::writer_thread.cancel();
    if (share::input_thread.is_initialized() && share::input_thread.is_alive())
        share::input_thread.cancel();

    share::run_gui = false;
}

} // namespace client
