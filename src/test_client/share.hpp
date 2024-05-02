#pragma once

// common
#include "../common/threading.hpp"
#include "../common/types.hpp"

// std
#include <atomic>
#include <queue>
#include <string>

namespace test_client::share {

extern bool only_drawing;
extern std::string username;
extern std::atomic_uint32_t expected_responses;

extern threading::thread reader_thread;
extern threading::thread writer_thread;

extern threading::mutex writer_mutex;
extern threading::cond_var writer_cond;
extern std::queue<Action> writer_queue;

extern TaggedDrawVector tagged_draw_vector;

} // namespace test_client::share
