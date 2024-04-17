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
        uint32_t ipv4_addr, uint16_t port);

    void operator()();

private:
    static void setup_logging();
    static void close_logging();

    bool setup_connection();
    void close_connection();

    void setup_writer_thread();
    void close_writer_thread();

    void setup_reader_thread();
    void close_reader_thread();

    // info
    const uint32_t m_ipv4_addr;
    const uint16_t m_port;

    // socket
    int m_conn_fd {};

    // writer thread
    std::thread m_writer_thread {};

    // log file
    static common::log_file_t s_log_file;
};

} // namespace client
