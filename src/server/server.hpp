#pragma once

// common
#include <log.hpp>

namespace server {

class server_t {
public:
    explicit server_t(uint16_t port);

    int operator()();

private:
    void requests_loop();

    uint16_t m_port { 0 };

    int m_socket_fd { 0 };
};

} // namespace server
