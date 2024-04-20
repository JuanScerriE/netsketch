#pragma once

// std
#include <list>
#include <unordered_set>

// common
#include <draw_list.hpp>
#include <types.hpp>

// server
#include <event.hpp>

// protocol
#include <protocol.hpp>

// threading
#include <threading.hpp>

namespace server::share {

extern threading::mutex e_threads_mutex;

extern std::list<threading::pthread> e_threads;

extern threading::pthread e_updater_thread;

extern threading::pthread e_server_thread;

extern threading::mutex e_connections_mutex;

extern std::unordered_set<int> e_connections;

extern common::ts_queue<prot::tagged_command_t>
    e_command_queue;

extern common::ts_draw_list e_draw_list;

} // namespace server::share
