#pragma once

// common
#include "../common/threading.hpp"
#include "../common/types.hpp"

// std
#include <queue>
#include <string>

namespace test_client::share {

extern bool only_drawing;

extern threading::thread reader_thread;
extern threading::thread writer_thread;

extern std::string username;

extern threading::mutex writer_mutex;
extern threading::cond_var writer_cond;
extern std::queue<Action> writer_queue;

extern uint32_t expected_responses;

} // namespace test_client::share
