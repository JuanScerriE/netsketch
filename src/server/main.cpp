// server
#include <server.hpp>

// common
#include <logger.hpp>

// unix
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

// std
#include <csignal>

// share
#include <share.hpp>

// cli11
#include <CLI/CLI.hpp>

#define SERVER_PORT (6666)


void sigint_handler(int) {
    pollfd poll_fd{
        server::share::e_stop_event_fd,
        POLLOUT,
        0
    };

    if (poll(&poll_fd, 1, -1) == -1) {
        AbortV(
            "poll of event file descriptor failed, reason: "
            "{}",
            strerror(errno)
        );
    }

    if (!(poll_fd.revents & POLLOUT)) {
        Abort("cannot write to event file descriptor");
    }

    uint64_t inc = 1;

    if (write(
            server::share::e_stop_event_fd,
            &inc,
            sizeof(uint64_t)
        ) == -1) {
        AbortV(
            "failed to write to event file descriptor, "
            "reason: {}",
            strerror(errno)
        );
    }
}

int main(int argc, char** argv) {
    CLI::App app;

    uint16_t port{SERVER_PORT};
    app.add_option(
           "--port",
           port,
           "The port number of the NetSketch server"
    )
        ->capture_default_str();

    CLI11_PARSE(app, argc, argv);

    server::share::e_stop_event_fd = eventfd(0, 0);

    if (server::share::e_stop_event_fd == -1) {
        AbortV(
            "failed to create event file descriptor, "
            "reason: {}",
            strerror(errno)
        );
    }

    // NOTE: are using sigaction because the man page for
    // signal says so
    struct sigaction act {};

    bzero(&act, sizeof(act));

    act.sa_handler = sigint_handler;

    if (sigaction(SIGINT, &act, nullptr) == -1) {
        AbortV(
            "failed to create sigint handler, "
            "reason: {}",
            strerror(errno)
        );
    }

    server::server_t server{port};

    int status = server();

    if (close(server::share::e_stop_event_fd) == -1) {
        AbortV(
            "failed to close event file descriptor, reason "
            "{}",
            strerror(errno)
        );
    }

    return status;
}

// int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//
// if (socket_fd == -1) {
//     fmt::println(
//         stderr,
//         "warn: could not create socket, reason: {}",
//         strerror(errno)
//     );
//
//     return EXIT_FAILURE;
// }
//
// sockaddr_in server_addr{};
// server_addr.sin_family = AF_INET;
// server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
// server_addr.sin_port = htons(SERVER_PORT);
//
// if (bind(
//         socket_fd,
//         (sockaddr*)&server_addr,
//         sizeof(server_addr)
//     ) == -1) {
//     fmt::println(
//         stderr,
//         "warn: could not bind socket, reason: {}",
//         strerror(errno)
//     );
//
//     close(socket_fd);
//
//     return EXIT_FAILURE;
// }
//
// // NOTE: the backlog parameter limits the number
// // of outstanding connections on the socket's listen
// // queue.
// if (listen(socket_fd, 32) == -1) {
//     fmt::println(
//         stderr,
//         "warn: could not listen on socket, reason: {}",
//         strerror(errno)
//     );
//
//     close(socket_fd);
//
//     return EXIT_FAILURE;
// }
//
// // TODO: get the IP address and print it out
// fmt::println("server listening on port {}", SERVER_PORT);
//
// for (;;) {
//     sockaddr_in addr{};
//     socklen_t addr_len;
//
//     (void)addr;
//     (void)addr_len;
//
//     fmt::println(
//         "waiting for a connection on port {}",
//         SERVER_PORT
//     );
//
//     int conn_fd =
//         accept(socket_fd, (sockaddr*)&addr, &addr_len);
//
//     if (conn_fd == -1) {
//         fmt::println(
//             "warn: accepting incoming connection "
//             "failed, reason {}",
//             strerror(errno)
//         );
//
//         continue;
//     }
//
//     std::function<void(int, sockaddr_in)> conn_handler =
//         [](int conn_fd, sockaddr_in addr) {
//             char ip[INET_ADDRSTRLEN];
//
//             if (inet_ntop(
//                     AF_INET,
//                     &addr.sin_addr,
//                     ip,
//                     INET_ADDRSTRLEN
//                 ) == nullptr) {
//                 fmt::println(
//                     stderr,
//                     "warn: convert network readable ip "
//                     "address to text failed, reason {}",
//                     strerror(errno)
//                 );
//             }
//
//             fmt::println(
//                 "received connection from {}:{}",
//                 ip,
//                 ntohs(addr.sin_port)
//             );
//
//             ssize_t size =
//                 write(conn_fd, "hello", strlen("hello"));
//
//             if (size == -1) {
//                 fmt::println(
//                     stderr,
//                     "warn: writing to connection "
//                     "socket failed, reason {}",
//                     strerror(errno)
//                 );
//
//                 close(conn_fd);
//
//                 return;
//             }
// #undef _GNU_SOURCE
//             for (;;) {
// #ifdef _GNU_SOURCE
//                 pollfd poll_fd{
//                     conn_fd,
//                     POLLIN | POLLRDHUP,
//                     0
//                 };
// #else
//                 pollfd poll_fd{conn_fd, POLLIN, 0};
// #endif
//
//                 if (poll(&poll_fd, 1, 10 * MINUTE) == -1)
//                 {
//                     fmt::println(
//                         stderr,
//                         "warn: poll of connection "
//                         "failed, "
//                         "reason: {}",
//                         strerror(errno)
//                     );
//
//                     break;
//                 }
//
// #ifdef _GNU_SOURCE
//                 if (poll_fd.revents != 0) {
//                     fmt::println(
//                         stderr,
//                         "WAKE UP REASONS: ({}, {}, {}, "
//                         "{})",
//                         (poll_fd.revents & POLLIN)
//                             ? "POLLIN"
//                             : "",
//                         (poll_fd.revents & POLLHUP)
//                             ? "POLLHUP"
//                             : "",
//                         (poll_fd.revents & POLLRDHUP)
//                             ? "POLLRDHUP"
//                             : "",
//                         (poll_fd.revents & POLLERR)
//                             ? "POLLERR"
//                             : ""
//                     );
//                 }
// #else
//                 if (poll_fd.revents != 0) {
//                     fmt::println(
//                         stderr,
//                         "WAKE UP REASONS: ({}, {}, {})",
//                         (poll_fd.revents & POLLIN)
//                             ? "POLLIN"
//                             : "",
//                         (poll_fd.revents & POLLHUP)
//                             ? "POLLHUP"
//                             : "",
//                         (poll_fd.revents & POLLERR)
//                             ? "POLLERR"
//                             : ""
//                     );
//                 }
// #endif
//
//                 if (poll_fd.revents & POLLHUP) {
//                     fmt::println(
//                         stderr,
//                         "{}:{} initiated closing",
//                         ip,
//                         ntohs(addr.sin_port)
//                     );
//
//                     break;
//                 }
//
// #ifdef _GNU_SOURCE
//                 if (poll_fd.revents & POLLRDHUP) {
//                     fmt::println(
//                         stderr,
//                         "{}:{} user initiated closing",
//                         ip,
//                         ntohs(addr.sin_port)
//                     );
//
//                     shutdown(conn_fd, SHUT_RDWR);
//
//                     continue;
//                 }
//
// #else
//                 if ((poll_fd.revents & POLLIN)) {
//                     char buf[1024];
//
//                     bzero(buf, 1024);
//
//                     if (read(conn_fd, buf, 1024 - 1) ==
//                         -1) {
//                         fmt::println(
//                             stderr,
//                             "reading from connection "
//                             "failed, reason: "
//                             "{}",
//                             strerror(errno)
//                         );
//                     } else {
//                         if (strlen(buf) == 0) {
//                             // check if it is alive
//                             // if I get not response
//                             // then its dead.
//
//                             pollfd potentially_dead_fd{
//                                 conn_fd,
//                                 POLLOUT,
//                                 0
//                             };
//
//                             if (poll(
//                                     &potentially_dead_fd,
//                                     1,
//                                     MINUTE /
//                                         2  // 30 seconds
//                                 ) == -1) {
//                                 fmt::println(
//                                     stderr,
//                                     "warn: poll for "
//                                     "potentially dead "
//                                     "connection "
//                                     "failed, "
//                                     "reason: {}",
//                                     strerror(errno)
//                                 );
//
//                                 break;
//                             }
//
//                             if (!(poll_fd.revents &
//                             POLLOUT
//                                 )) {
//                                 fmt::println(
//                                     stderr,
//                                     "poll interrupted "
//                                     "and cannot write "
//                                     "to "
//                                     "{}:{}",
//                                     ip,
//                                     ntohs(addr.sin_port)
//                                 );
//
//                                 break;
//                             }
//
//                             if (write(
//                                     conn_fd,
//                                     "Are you alive?",
//                                     strlen("Are you
//                                     alive?")
//                                 ) == -1) {
//                                 fmt::println(
//                                     stderr,
//                                     "cannot write to "
//                                     "{}:{}",
//                                     ip,
//                                     ntohs(addr.sin_port)
//                                 );
//
//                                 break;
//                             }
//
//                             potentially_dead_fd = {
//                                 conn_fd,
//                                 POLLIN,
//                                 0
//                             };
//
//                             if (poll(
//                                     &potentially_dead_fd,
//                                     1,
//                                     MINUTE /
//                                         2  // 30 seconds
//                                 ) == -1) {
//                                 fmt::println(
//                                     stderr,
//                                     "warn: poll for "
//                                     "potentially dead "
//                                     "connection "
//                                     "failed, "
//                                     "reason: {}",
//                                     strerror(errno)
//                                 );
//
//                                 break;
//                             }
//
//                             if (!(poll_fd.revents &
//                             POLLIN
//                                 )) {
//                                 fmt::println(
//                                     stderr,
//                                     "poll interrupted "
//                                     "and cannot read "
//                                     "from {}:{}",
//                                     ip,
//                                     ntohs(addr.sin_port)
//                                 );
//
//                                 break;
//                             }
//
//                             char buf[1024];
//
//                             bzero(buf, 1024);
//
//                             if (read(
//                                     conn_fd,
//                                     buf,
//                                     1024 - 1
//                                 ) == -1) {
//                                 fmt::println(
//                                     stderr,
//                                     "cannot read from "
//                                     "{}:{}",
//                                     ip,
//                                     ntohs(addr.sin_port)
//                                 );
//
//                                 break;
//                             }
//
//                             if (std::string{buf} !=
//                                 "Alive and kickin'") {
//                                 break;
//                             }
//                         }
//                     }
//                 }
// #endif
//
// #define _GNU_SOURCE
//
//                 if (!(poll_fd.revents & POLLIN)) {
//                     fmt::println(
//                         stderr,
//                         "server initiated closing of "
//                         "connection {}:{}",
//                         ip,
//                         ntohs(addr.sin_port)
//                     );
//
//                     break;
//                 }
//             }
//
//             close(conn_fd);
//
//             fmt::println(
//                 stderr,
//                 "closing: connection {}:{}",
//                 ip,
//                 ntohs(addr.sin_port)
//             );
//         };
//
//     auto _ = std::async(
//         std::launch::async,
//         conn_handler,
//         conn_fd,
//         addr
//     );
// }
//
// return EXIT_SUCCESS;
// }
