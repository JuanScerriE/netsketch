#pragma once

// std
#include <string>

// cstd
#include <cstdint>

// common
#include "../common/channel.hpp"
#include "../common/network.hpp"

namespace test_client {

class Runner {
   public:
    Runner() = default;

    bool setup(
        const std::string& ipv4_addr,
        uint16_t port,
        uint32_t iterations,
        double interval,
        uint32_t expected_responses,
        const std::string& username,
        bool other_actions
    );

    [[nodiscard]] bool run() const;

    ~Runner();

   private:
    uint32_t m_ipv4_addr {};
    uint16_t m_port {};
    uint32_t m_iterations {};
    double m_interval {};
    IPv4Socket m_sock {};
    Channel m_channel {};
};

} // namespace test_client
