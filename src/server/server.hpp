#pragma once

// std
#include <optional>

// common
#include "../common/channel.hpp"
#include "../common/log.hpp"
#include "../common/network.hpp"

namespace server {

class Server {
   public:
    explicit Server(uint16_t port);

    void operator()();

   private:
    void request_loop();

    std::optional<std::string> is_valid_username(Channel channel);

    IPv4Socket m_sock {};

    uint16_t m_port {};
};

} // namespace server
