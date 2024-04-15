// client
#include <network_handler.hpp>

// common
#include <log_file.hpp>

#include "log.hpp"
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
common::log_file_t network_handler_t::s_log_file{};

network_handler_t::network_handler_t(
    uint32_t ipv4_addr,
    uint16_t port,
    std::string nickname
)
    : m_ipv4_addr(ipv4_addr),
      m_port(port),
      m_nickname(std::move(nickname)) {
}

void network_handler_t::operator()() {
    log::disable();

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
        log::set_file(s_log_file);

        log::set_level(log::level::debug);
    }

    handle_loop();

    if (s_log_file.is_open()) {
        s_log_file.close();
    }
}

void network_handler_t::handle_loop() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        log::error(
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
        log::error(
            "could not connect, reason: {}",
            strerror(errno)
        );

        close(socket_fd);

        return;
    }

    constexpr nfds_t nfds{2};

    constexpr size_t socket_idx{0};
    constexpr size_t event_idx{1};

    for (;;) {
        pollfd poll_fds[nfds] = {
            {socket_fd, POLLIN, 0},
            {share::e_network_thread_event->read_fd(),
             POLLIN,
             0},
        };

        if (poll(poll_fds, nfds, -1) == -1) {
            log::error(
                "poll of connection failed, reason: {}",
                strerror(errno)
            );

            break;
        }

        log::debug(
            "wake up reasons ({}, {}, {})",
            (poll_fds[socket_idx].revents & POLLIN)
                ? "POLLIN"
                : "",
            (poll_fds[socket_idx].revents & POLLHUP)
                ? "POLLHUP"
                : "",
            (poll_fds[socket_idx].revents & POLLERR)
                ? "POLLERR"
                : ""
            // endif
        );

        if (poll_fds[socket_idx].revents & POLLHUP) {
            log::info("closing tcp connection");

            break;
        }

        if ((poll_fds[event_idx].revents & POLLIN)) {
            log::info("initiated closing");

            if (shutdown(socket_fd, SHUT_WR) == -1) {
                log::error(
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno)
                );
            }

            continue;
        }

        AbortIf(
            !(poll_fds[socket_idx].revents & POLLIN),
            "expected POLLIN"
        );

        char buf[1024];

        bzero(buf, 1024);

        ssize_t size = read(socket_fd, buf, 1024 - 1);

        if (size == -1) {
            log::warn(
                "reading from connection failed, reason: "
                "{}",
                strerror(errno)
            );

            continue;
        }

        if (size == 0) {
            // we will assume that if size == 0 and the poll
            // returns POLLIN that the write end of the
            // connection was close
            log::warn(
                "input of length 0 bytes (server closed "
                "write)"
            );

            if (shutdown(socket_fd, SHUT_WR) == -1) {
                log::error(
                    "connection shutdown failed, reason: "
                    "{}",
                    strerror(errno)
                );
            }

            continue;
        }

        log::info("response: {}", buf);
    }

    if (close(socket_fd) == -1) {
        AbortV(
            "closing connection failed, reason: {}",
            strerror(errno)
        );
    }
}

}  // namespace client
