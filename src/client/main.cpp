// client
#include <command_input.hpp>
#include <gui.hpp>
#include <log_file.hpp>

// std
#include <atomic>
#include <iostream>
#include <thread>

std::atomic_bool stop{false};

int main() {
     std::thread gui_thread(client::gui_t{stop});

     client::command_input_t command_input;

     command_input.start();

     stop = true;

     gui_thread.join();

    return 0;
}
