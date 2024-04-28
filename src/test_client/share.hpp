#pragma once

// common
#include "../common/log_file.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// std
#include <queue>
#include <string>

namespace client::share {

extern threading::thread reader_thread;
extern threading::thread writer_thread;

extern logging::log_file log_file;

extern std::string username;

extern threading::mutex writer_mutex;

extern threading::cond_var writer_cond;

extern std::queue<Action> writer_queue;

} // namespace client::share
