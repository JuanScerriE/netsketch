#pragma once

// std
#include <string_view>

namespace client {

class command_input_t {
   public:
    command_input_t(std::atomic_bool& stop);

    void start();

    void operator()();

   private:
    void process_line(std::string_view line_view);

    bool m_should_exit{false};

    std::atomic_bool& a_stop;
};

}  // namespace client
