#pragma once

// common
#include "../common/log.hpp"
#include "../common/network.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// std
#include <atomic>
#include <list>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

// server
#include "timer_data.hpp"

namespace server::share {

extern std::atomic_bool run;

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

extern logging::log timing_log;

} // namespace server::share
