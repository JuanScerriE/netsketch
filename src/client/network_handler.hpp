#pragma once

// common
#include <cstdint>
#include <logger.hpp>
#include <types.hpp>

// client
#include <gui.hpp>

// std
#include <vector>
#include <cstdint>

namespace client {

class network_handler_t {
   public:
    explicit network_handler_t(
        uint32_t ipv4_addr,
        uint16_t port,
        std::string nickname,
        bool log = true
    );

    void operator()();

   private:
    void handle_loop();

    // info
    const uint32_t m_ipv4_addr;
    const uint16_t m_port;
    const std::string m_nickname;
    std::string m_internal_nickname{};

    // file logging
    bool m_log{true};
    static common::logger_t s_logger;
    static common::log_file_t s_log_file;

    // draws
    std::vector<common::draw_t> m_draws{};
};

}  // namespace client
