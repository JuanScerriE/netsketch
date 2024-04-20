#pragma once

// common
#include <log.hpp>

namespace server {

class server_t {
public:
    explicit server_t(uint16_t port);

    void operator()();

    void dtor();

private:
    void requests_loop();

    uint16_t m_port { 0 };

    int m_socket_fd { 0 };

    int m_current_conn_fd { -1 };

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace server
