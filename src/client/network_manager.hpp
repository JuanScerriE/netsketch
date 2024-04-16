#pragma once

// common
#include <log_file.hpp>
#include <types.hpp>

// std
#include <thread>

namespace client {

class network_manager_t {
   public:
    explicit network_manager_t(
        uint32_t ipv4_addr,
        uint16_t port
    );

    void operator()();

   private:
    void setup_logging();
    void close_logging();

    void setup_connection();
    void close_connection();

    void setup_writer_thread();
    void close_writer_thread();

    void setup_reader_thread();
    void close_reader_thread();

    // info
    const uint32_t m_ipv4_addr;
    const uint16_t m_port;

    // socket
    int m_conn_fd{};

    // writer thread
    static std::thread m_writer_thread;

    // log file
    common::log_file_t m_log_file{};
};

}  // namespace client
