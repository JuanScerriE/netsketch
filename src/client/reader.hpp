#pragma once

// common
#include "../common/channel.hpp"
#include "../common/log.hpp"
#include "../common/types.hpp"

namespace client {

class Reader {
   public:
    explicit Reader(const Channel& channel);

    void operator()();

    void shutdown();

   private:
    void read_loop();

    void handle_payload(ByteString& payload);

    void update_list(TaggedAction& tagged_command);
    void update_whole_list(TaggedDrawVector& list);

    Channel m_channel;

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace client
