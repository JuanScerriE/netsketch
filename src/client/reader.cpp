// client
#include "reader.hpp"
#include "share.hpp"

// common
#include "../common/log.hpp"
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

namespace client {

Reader::Reader(const Channel& channel)
    : m_channel(channel)
{
}

void Reader::operator()()
{
    setup_logging();

    read_loop();
}

void Reader::read_loop()
{
    for (;;) {
        auto [res, status] = m_channel.read();

        if (status != ChannelErrorCode::OK) {
            log.error("reading failed, reason {}", status.what());

            break;
        }

        handle_payload(res);

        log.flush();
    }

    shutdown();
}

void Reader::handle_payload(ByteVector& bytes)
{
    log.debug("received: 0x{}", bytes_to_string(bytes));

    Deserialize deserializer { bytes };

    auto [payload, status] = deserializer.payload();

    if (status != DeserializeErrorCode::OK) {
        log.warn("deserialization failed, reason {}", status.what());

        return;
    }

    std::visit(
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
            },
            [](auto& object) {
                log.warn("unexpected payload type {}", typeid(object).name());
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
    if (share::input_thread.is_initialized() && share::input_thread.is_alive())
        share::input_thread.cancel();

    share::run_gui = false;
}

logging::log Reader::log {};

void Reader::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix("[reader]");

    log.set_file(share::log_file);
}

} // namespace client
