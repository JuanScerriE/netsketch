#pragma once

// common
#include "../common/channel.hpp"
#include "../common/log.hpp"

namespace client {

class Writer {
   public:
    explicit Writer(Channel channel);

    void operator()();

    void shutdown();

   private:
    // connection
    Channel m_channel {};
};

} // namespace client
