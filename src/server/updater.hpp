#pragma once

// common
#include "../common/channel.hpp"
#include "../common/log.hpp"
#include "../common/threading.hpp"

// server
#include "share.hpp"

namespace server {

class Updater {
   public:
    [[noreturn]] void operator()()
    {
        setup_logging();

        for (;;) {
            Payload payload {};

            {
                threading::unique_mutex_guard guard { share::update_mutex };

                share::update_cond.wait(guard, []() {
                    return share::payload_queue.empty();
                });

                payload = share::payload_queue.back();

                share::payload_queue.pop();
            }

            ByteVector bytes = Serialize { payload }.bytes();

            {
                threading::mutex_guard guard { share::connections_mutex };

                for (auto& [fd, conn] : share::connections) {
                    Channel channel { conn };

                    auto status = channel.write(bytes);

                    if (status != ChannelErrorCode::OK) {
                        log.error("[{}] writing failed, reason {}", fd, status.what());
                    }
                }
            }
        }
    }

   private:
    static inline logging::log log {};

    static void setup_logging()
    {
        using namespace logging;

        log.set_level(log::level::debug);

        log.set_prefix("[updater]");
    }
};

} // namespace server
