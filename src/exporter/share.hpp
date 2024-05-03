#pragma once

// common
#include "../common/threading.hpp"
#include "../common/types.hpp"

// std
#include <queue>
#include <string>

namespace exporter::share {

extern threading::thread reader_thread;
extern threading::thread writer_thread;
extern threading::thread input_thread;

extern std::string username;

extern bool show_mine;

extern bool run_gui;

extern threading::mutex writer_mutex;
extern threading::cond_var writer_cond;
extern std::queue<Action> writer_queue;

// double instance locking state
// (http://concurrencyfreaks.blogspot.com/2013/11/double-instance-locking.html)

extern threading::mutex tagged_draw_vector_mutex;

extern threading::rwlock rwlock1;
extern threading::rwlock rwlock2;

extern TaggedDrawVector vec1;
extern TaggedDrawVector vec2;

} // namespace client::share
