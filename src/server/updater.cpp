// server
#include "updater.hpp"
#include "share.hpp"

// common
#include "../common/channel.hpp"
#include "../common/tagged_draw_vector_wrapper.hpp"
#include "../common/threading.hpp"

// bench
#include "../bench/bench.hpp"

// spdlog
#include <spdlog/spdlog.h>

// std
#include <variant>

namespace server {

void Updater::operator()()
{
    for (;;) {
        Payload payload {};

        {
            threading::unique_mutex_guard guard { share::update_mutex };

            share::update_cond.wait(guard, []() {
                return share::payload_queue.empty();
            });

            BENCH("updater reading changes");

            payload = share::payload_queue.back();

            share::payload_queue.pop();

            if (std::holds_alternative<Adopt>(payload)) {
                TaggedDrawVectorWrapper { share::tagged_draw_vector }.adopt(
                    std::get<Adopt>(payload)
                );
            } else if (std::holds_alternative<TaggedAction>(payload)) {
                TaggedDrawVectorWrapper { share::tagged_draw_vector }.update(
                    std::get<TaggedAction>(payload)
                );
            } else {
                ABORT("unreachable");
            }
        }

        ByteString bytes = serialize<Payload>(payload);

        {
            BENCH("updating all connected clients");

            threading::mutex_guard guard { share::connections_mutex };

            for (auto& [fd, conn] : share::connections) {
                Channel channel { conn };

                auto status = channel.write(bytes);

                if (status != ChannelErrorCode::OK) {
                    spdlog::error(
                        "[{}] writing failed, reason {}",
                        fd,
                        status.what()
                    );
                }
            }

            // end of bench scope (for clarity)
        }
    }
}

} // namespace server
