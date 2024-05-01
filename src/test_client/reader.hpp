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
    uint32_t m_responses{0};

    std::size_t m_hash{0};

    Channel m_channel;
};

} // namespace test_client
