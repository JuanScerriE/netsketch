#pragma once

// unix
#include <netinet/in.h>

// common
#include "../common/bytes.hpp"
#include "../common/channel.hpp"
#include "../common/log.hpp"
#include "../common/network.hpp"

namespace server {

class ConnHandler {
   public:
    explicit ConnHandler(IPv4SocketRef sock, std::string username);

    void operator()();

   private:
    void setup_readable_net_info();

    bool send_full_list();

    void handle_payload(const ByteString& bytes);

    IPv4SocketRef m_sock {};

    Channel m_channel {};

    std::string m_ipv4 {};
    std::string m_port {};

    std::string m_username {};
};

} // namespace server
