#pragma once

// common
#include <log.hpp>

#include <network.hpp>
#include <unordered_set>

namespace server {

class server_t {
   public:
    explicit server_t(uint16_t port);

    void operator()();

    void dtor();

   private:
    void request_loop();

    uint16_t m_port { 0 };

    IPv4Socket m_sock {};

    std::unordered_set<std::string> m_users {};

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace server
