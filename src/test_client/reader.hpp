#pragma once

// common
#include "../common/channel.hpp"
#include "../common/types.hpp"

// std
// #include <atomic>

namespace test_client {

class Reader {
   public:
    explicit Reader(const Channel& channel);

    void operator()();

    void shutdown();

   private:
    void handle_payload(ByteString& bytes);

    // std::atomic_uint32_t m_responses { 0 };

    Channel m_channel;
};

} // namespace test_client
