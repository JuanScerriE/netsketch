#pragma once

// common
#include <log.hpp>

#include <unordered_set>
#include <network.hpp>

namespace server {

class server_t {
public:
    explicit server_t(uint16_t port);

    void operator()();

    void dtor();

private:
    void requests_loop();

    uint16_t m_port { 0 };

    Socket m_socket{};

    int m_current_conn_fd { -1 };

    std::unordered_set<std::string> m_users {};

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace server
