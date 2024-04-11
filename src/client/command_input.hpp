#pragma once

// std
#include <string_view>

namespace client {

class command_input_t {
   public:
    void start();

   private:
    void process_line(std::string_view line_view);

    bool should_exit();
};

}  // namespace client
