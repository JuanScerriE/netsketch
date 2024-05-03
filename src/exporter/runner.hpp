#pragma once

// std
#include <string>

// cstd
#include <cstdint>

// common
#include "../common/channel.hpp"
#include "../common/network.hpp"
#include "../common/types.hpp"

namespace exporter {

class Runner {
   public:
    Runner() = default;

    bool setup(
        const std::string& username,
        const std::string& ipv4_addr,
        uint16_t port
    );

    [[nodiscard]] bool run();

    [[nodiscard]] bool generate_image(TaggedDrawVector& draws);

    ~Runner();

   private:
    std::string m_username {};
    uint32_t m_ipv4_addr {};
    uint16_t m_port {};
    IPv4Socket m_sock {};
    Channel m_channel {};
};

} // namespace client
