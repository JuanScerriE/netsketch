#pragma once

// unix
#include <netinet/in.h>

// common
#include <logger.hpp>

namespace server {

class conn_handler_t {
   public:
    explicit conn_handler_t(
        int conn_fd,
        sockaddr_in addr,
        common::logger_t& logger
    );

    void operator()();

   private:
    bool is_still_alive();

    template <typename... T>
    void slog(  // specific log
        common::level_e level,
        fmt::format_string<T...> fmt,
        T&&... args
    ) {
        m_logger.log(
            level,
            "[{}:{}] {}",
            m_ipv4,
            m_port,
            fmt::format(fmt, args...)
        );
    }

    int m_conn_fd;

    // network info
    sockaddr_in m_addr;
    std::string m_ipv4{};
    uint16_t m_port{};

    // for know this won't be atomic
    // if it needs to be atomic then we'll do
    // something about it.
    common::logger_t& m_logger;
};

}  // namespace server
