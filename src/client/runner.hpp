#pragma once

// std
#include <string>

// cstd
#include <cstdint>

// common
#include "../common/channel.hpp"
#include "../common/network.hpp"

namespace client {

class Runner {
   public:
    Runner() = default;

    bool setup(
        bool use_gui,
        const std::string& username,
        const std::string& ipv4_addr,
        uint16_t port
    );

    [[nodiscard]] bool run() const;

    ~Runner();

   private:
    bool m_use_gui {};
    uint32_t m_ipv4_addr {};
    uint16_t m_port {};
    IPv4Socket m_sock {};
    Channel m_channel {};
};

} // namespace client
