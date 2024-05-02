#pragma once

// cstd
#include <ctime>

// common
#include "../common/timer.hpp"

namespace server {

struct TimerData {
    Timer timer {};
    std::string username {};
};

void handle_timer(union sigval val);

void create_client_timer(const std::string& username);

} // namespace server
