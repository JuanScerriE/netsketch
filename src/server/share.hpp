#pragma once

// common
#include "../common/network.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"
#include "../common/log.hpp"

// std
#include <atomic>
#include <list>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

// server
#include "timer_data.hpp"

// I think 64 connections is a reasonable number of
// connections for a whiteboard (for know).
#define MAX_CONNS (3)

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
