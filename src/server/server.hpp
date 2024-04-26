#pragma once

// std
#include <optional>
#include <unordered_set>

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

    std::unordered_set<std::string> m_users {};

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace server
