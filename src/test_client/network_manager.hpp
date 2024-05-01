#pragma once

// common
#include "../common/channel.hpp"
#include "../common/network.hpp"

namespace test_client {

class NetworkManager {
   public:
    explicit NetworkManager(uint32_t ipv4, uint16_t port);

    bool setup();
    void close();

    ~NetworkManager();

   private:
    bool open_socket();
    void wrap_socket();
    bool send_username();
    void start_reader();
    void start_writer();
    void close_reader();
    void close_writer();

    // info
    const uint32_t m_ipv4;
    const uint16_t m_port;

    IPv4Socket m_conn {};

    Channel m_channel {};
};

} // namespace test_client
