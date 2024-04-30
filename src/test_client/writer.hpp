#pragma once

// common
#include "../common/channel.hpp"

namespace test_client {

class Writer {
   public:
    explicit Writer(Channel channel);

    void operator()();

    void shutdown();

   private:
    // connection
    Channel m_channel {};
};

} // namespace test_client
