#pragma once

// common
#include <logger.hpp>

namespace server {

class server_t {
   public:
    explicit server_t(
        uint16_t port,
        common::level_e log_level = common::level_e::DEBUG
    );

    int operator()();

   private:
    void requests_handler();

    uint16_t m_port{};

    int m_socket_fd{0};

    common::logger_t m_logger{};

};

}  // namespace server
