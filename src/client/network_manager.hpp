#pragma once

// logging
#include <log.hpp>

namespace client {

class network_manager_t {
   public:
    explicit network_manager_t(
        uint32_t ipv4_addr,
        uint16_t port
    );

    bool setup();
    void close();

   private:
    bool setup_connection();
    void close_connection();

    void setup_reader_thread();
    void close_reader_thread();

    void setup_writer_thread();
    void close_writer_thread();

    // info
    const uint32_t m_ipv4_addr;
    const uint16_t m_port;

    // socket
    int m_conn_fd {};

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace client
