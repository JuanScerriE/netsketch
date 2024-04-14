// client
#include <gui.hpp>
#include <input_handler.hpp>
#include <log_file.hpp>
#include <network_handler.hpp>

// common
#include <types.hpp>

// std
#include <regex>
#include <thread>

// unix
#include <arpa/inet.h>
#include <sys/eventfd.h>
#include <unistd.h>

// cli11
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>

// share
#include <share.hpp>

struct IPv4Validator : public CLI::Validator {
    IPv4Validator() {
        name_ = "IPv4";
        func_ = [](const std::string &str) {
            std::regex ipv4_regex{
                R"(^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}(25[0-5]|(2[0-4]|1\d|[1-9]|)\d)$)"
            };

            if (!std::regex_match(str, ipv4_regex))
                return std::string{
                    "string is not a IPv4 address"
                };
            else
                return std::string{};
        };
    }
};

const static IPv4Validator IPv4;

int main(int argc, char **argv) {
    CLI::App app;

    std::string ipv4_addr{"127.0.0.1"};
    app.add_option(
           "--server",
           ipv4_addr,
           "The IPv4 address of a machine hosting a "
           "NetSketch server"
    )
        ->capture_default_str()
        ->check(IPv4);

    uint16_t port{6666};
    app.add_option(
           "--port",
           port,
           "The port number of a NetSketch server"
    )
        ->capture_default_str();

    std::string nickname{};
    app.add_option(
           "--nickname",
           nickname,
           "The nickname of the user (used for "
           "identification on a NetSketch server)"
    )
        ->required();

    CLI11_PARSE(app, argc, argv);

    in_addr addr{};

    if (inet_pton(AF_INET, ipv4_addr.c_str(), &addr) <= 0) {
        // NOTE: not using AbortIf since the above is
        // actually causing a mutation, so I just
        // want that to be clear
        Abort("invalid IPv4 address");
    }

    // NOTE: this is in human readable form
    uint32_t ipv4_addr_int = ntohl(addr.s_addr);

    client::share::e_network_thread_event_fd =
        eventfd(0, 0);

    if (client::share::e_network_thread_event_fd == -1) {
        AbortV(
            "creation of event file descriptor for "
            "notifications failed, reason {}",
            strerror(errno)
        );
    }

    // this is the first since the
    // network handler and the input handler both
    // have a reference to the gui
    client::gui_t gui{};

    std::thread network_thread(client::network_handler_t{
        ipv4_addr_int,
        port,
        nickname
    });

    // the input thread has the main control
    std::thread input_thread(client::input_handler_t{});

    // NOTE: we are running the GUI in the main
    // thread because of macOS, that is macOS
    // does not like the GLFW which raylib uses
    // running in a separate thread
    gui();

    input_thread.join();
    network_thread.join();

    // gui closes last with the end of the program

    if (close(client::share::e_network_thread_event_fd) ==
        -1) {
        AbortV(
            "failed to close event file descriptor, reason "
            "{}",
            strerror(errno)
        );
    }

    return 0;
}
