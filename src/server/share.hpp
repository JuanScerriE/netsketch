#pragma once

// common
#include "draw_list.hpp"
#include <types.hpp>

// server
#include <event.hpp>

// protocol
#include <protocol.hpp>

namespace server::share {

extern common::event_t* e_stop_event;

extern std::vector<int> e_connections;

extern common::queue_st<prot::tagged_command_t>
    e_command_queue;

extern common::ts_draw_list e_draw_list;

} // namespace server::share
