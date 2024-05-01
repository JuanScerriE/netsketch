#pragma once

// common
#include "../common/channel.hpp"
#include "../common/types.hpp"

namespace test_client {

class Reader {
   public:
    explicit Reader(const Channel& channel);

    void operator()();

    void shutdown();

   private:
    void handle_payload(ByteString& bytes);

    Channel m_channel;
};

} // namespace test_client
