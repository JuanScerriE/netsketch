// client
#include <command_input.hpp>
#include <gui.hpp>
#include <log_file.hpp>

// std
#include <atomic>
#include <thread>

std::atomic_bool stop{false};

int main() {
    client::gui_t gui{stop};

    client::command_input_t command_input{stop};

    std::thread input_thread(client::command_input_t{stop});

    gui();

    input_thread.join();

    return 0;
}
