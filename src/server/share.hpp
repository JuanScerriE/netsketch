#pragma once

// common
#include "../common/network.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// std
#include <list>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

// spdlog
#include <spdlog/logger.h>

// server
#include "timer_data.hpp"

namespace server::share {

// NOTE: this namespace contains all the globals the
// application uses. These globals have been
// logically grouped according to their specifically
// mutexes and conditions variables have been grouped
// with the actual structures they protect

extern threading::mutex users_mutex;
extern std::unordered_set<std::string> users;

extern threading::mutex threads_mutex;
extern std::list<threading::thread> threads;
extern threading::thread updater_thread;

extern threading::mutex connections_mutex;
extern std::unordered_map<int, IPv4Socket> connections;

extern threading::mutex timers_mutex;
extern std::list<std::unique_ptr<TimerData>> timers;

extern threading::mutex update_mutex;
extern threading::cond_var update_cond;
extern TaggedDrawVector tagged_draw_vector;
extern std::queue<Payload> payload_queue;

extern float time_out;

} // namespace server::share
