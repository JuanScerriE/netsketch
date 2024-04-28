#pragma once

// common
#include "../common/bench.hpp"
#include "../common/channel.hpp"
#include "../common/threading.hpp"

// server
#include "share.hpp"

// spdlog
#include <spdlog/spdlog.h>

namespace server {

class Updater {
   public:
    [[noreturn]] void operator()()
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
            }
        }
    }
};

} // namespace server
