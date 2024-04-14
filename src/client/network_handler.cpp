// client
#include <network_handler.hpp>

// common
#include <log_file.hpp>

#include "logger.hpp"
#include "share.hpp"

// fmt
#include <fmt/chrono.h>
#include <fmt/core.h>

// unix (hopefully)
#include <netinet/in.h>
#include <poll.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

namespace client {

// class statics
common::logger_t network_handler_t::s_logger{};
common::log_file_t network_handler_t::s_log_file{};

network_handler_t::network_handler_t(
    uint32_t ipv4_addr,
    uint16_t port,
    std::string nickname,
    bool log
)
    : m_ipv4_addr(ipv4_addr),
      m_port(port),
      m_nickname(std::move(nickname)),
      m_log(log) {
}

void network_handler_t::operator()() {
    // AbortIf(
    //     m_in_game_loop,
    //     "calling operator()() twice on a gui_t object"
    // );

    s_logger.disable();

    if (m_log) {
        using std::chrono::system_clock;

        auto now = system_clock::now();

        s_log_file.open(fmt::format(
            "netsketch-client-net-handler-log {:%Y-%m-%d "
            "%H:%M:%S}",
            now
        ));

        if (s_log_file.error()) {
            fmt::println(
                stderr,
                "warn: opening a log file failed because - "
                "{}",
                s_log_file.reason()
            );
        } else {
            s_logger.set_file(s_log_file);

            s_logger.set_level(common::level_e::DEBUG);
        }
    }

    handle_loop();

    if (m_log) {
        if (s_log_file.is_open()) {
            s_log_file.close();
        }
    }
}

void network_handler_t::handle_loop() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        s_logger.log(
            common::level_e::ERROR,
            "could not create socket, reason: {}",
            strerror(errno)
        );

        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(m_ipv4_addr);
    server_addr.sin_port = htons(m_port);

    if (connect(
            socket_fd,
            (sockaddr *)&server_addr,
            sizeof(server_addr)
        ) == -1) {
        s_logger.log(
            common::level_e::ERROR,
            "could not connect, reason: {}",
            strerror(errno)
        );

        close(socket_fd);

        return;
    }

    constexpr nfds_t nfds{2};

    constexpr size_t socket_idx{0};
    constexpr size_t event_idx{1};

    pollfd poll_fds[nfds] = {
        {socket_fd, POLLIN, 0},
        {share::e_network_thread_event_fd, POLLIN, 0},
    };

    for (;;) {
        if (poll(poll_fds, nfds, -1) == -1) {
            s_logger.log(
                common::level_e::ERROR,
                "poll of connection failed, reason: {}",
                strerror(errno)
            );

            close(socket_fd);

            return;
        }

        if (poll_fds[event_idx].revents & POLLIN) {
            break;
        }

        if (poll_fds[socket_idx].revents != 0) {
            s_logger.log(
                common::level_e::INFO,
                "WAKE UP REASONS: ({}, {}, {})",
                (poll_fds[socket_idx].revents & POLLIN)
                    ? "POLLIN"
                    : "",
                (poll_fds[socket_idx].revents & POLLHUP)
                    ? "POLLHUP"
                    : "",
                (poll_fds[socket_idx].revents & POLLERR)
                    ? "POLLERR"
                    : ""
            );
        }

        char buf[1024];

        bzero(buf, 1024);

        if (read(socket_fd, buf, 1024 - 1) == -1) {
            s_logger.log(
                common::level_e::ERROR,
                "reading from connection failed, reason: "
                "{}",
                strerror(errno)
            );

            close(socket_fd);

            return;
        }

        s_logger.log(
            common::level_e::INFO,
            "THE SERVER TOLD US: {}",
            buf
        );
    }

    s_logger.log(
        common::level_e::DEBUG,
        "the network thread is cooked"
    );

    if (shutdown(socket_fd, SHUT_RDWR) == -1) {
        s_logger.log(
            common::level_e::ERROR,
            "connection shutdown failed, reason: "
            "{}",
            strerror(errno)
        );
    }

    close(socket_fd);
}

}  // namespace client
