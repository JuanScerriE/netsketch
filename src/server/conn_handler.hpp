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

private:
    void setup_readable_net_info();
    void send_full_list();
    void handle_payload(const util::byte_vector& payload);

    template <typename... T>
    void addr_log( // specific log
        log::level log_level, fmt::format_string<T...> fmt,
        T&&... args)
    {
        log::write(log_level, "[{}:{}] {}", m_ipv4, m_port,
            fmt::format(fmt, args...));
    }

    int m_conn_fd {};

    sockaddr_in m_addr {};

    std::string m_ipv4 {};

    uint16_t m_port {};
};

} // namespace server
