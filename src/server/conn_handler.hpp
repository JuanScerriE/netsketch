#pragma once

// unix
#include <netinet/in.h>

// common
#include <log.hpp>

// util
#include <utils.hpp>

namespace server {

class conn_handler_t {
   public:
    explicit conn_handler_t(int conn_fd, sockaddr_in addr);

    void operator()();

    void dtor();

   private:
    void setup_readable_net_info();

    void send_full_list();

    void handle_payload(const util::ByteVector& payload);

    int m_conn_fd {};

    sockaddr_in m_addr {};

    std::string m_ipv4 {};

    uint16_t m_port {};

    // logging
    static logging::log log;

    static void
    setup_logging(std::string ipv4, uint16_t port);
};

} // namespace server
