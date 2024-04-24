#pragma once

#include "protocol.hpp"
#include "threading.hpp"
#include <log.hpp>
#include <poll.h>
#include <share.hpp>
#include <variant>

namespace server {

class updater_t {
   public:
    [[noreturn]] void operator()()
    {
        setup_logging();

        for (;;) {
            // block until we get something
            auto command
                = share::e_command_queue.pop_front();

            // serialize the command
            prot::serialize_t serializer { command };

            util::byte_vector payload_bytes
                = serializer.bytes();

            // setup the header
            prot::PayloadHeader header { MAGIC_BYTES,
                                         payload_bytes.size(
                                         ) };

            // build the full packet
            util::byte_vector packet {};

            packet.reserve(
                sizeof(header) + payload_bytes.size()
            );

            for (auto& byte : util::to_bytes(header)) {
                packet.push_back(byte);
            }

            for (auto& byte : payload_bytes) {
                packet.push_back(byte);
            }

            log.debug(
                "payload size {}",
                payload_bytes.size()
            );

            log.debug("packet size {}", packet.size());

            {
                threading::mutex_guard guard {
                    share::e_connections_mutex
                };

                // NOTE: that the size of e_connections
                // might change under our feet
                for (int conn_fd : share::e_connections) {
                    pollfd conn_poll { conn_fd,
                                       POLLOUT,
                                       0 };

                    if (poll(&conn_poll, 1, -1) == -1) {
                        log.error(
                            "poll of connection failed, "
                            "reason: {}",
                            strerror(errno)
                        );

                        continue;
                    }

                    if (conn_poll.revents & POLLHUP) {
                        log.error("connection hung up");

                        continue;
                    }

                    if (write(
                            conn_fd,
                            packet.data(),
                            packet.size()
                        )
                        == -1) {
                        log.error(
                            "writing to connection failed, "
                            "reason: {}",
                            strerror(errno)
                        );

                        continue;
                    }
                }
            }
        }
    }

    void dtor()
    {
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
