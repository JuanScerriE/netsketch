#pragma once

// common
#include "../common/channel.hpp"

namespace test_client {

class Reader {
   public:
    explicit Reader(const Channel& channel);

    void operator()();

    void shutdown();

   private:
    void read_loop();

    Channel m_channel;
};

} // namespace test_client
