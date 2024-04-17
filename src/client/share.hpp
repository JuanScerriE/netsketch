#pragma once

// common
#include <event.hpp>
#include <types.hpp>

namespace client::share {

// NOTE: for these two variables below we
// are not using atomics since
// only one thread writes whilst the others
// only read additionally, since these are
// just primitive types there is not complexity
// in operations (might change these)

// used to stop the hole program
extern common::readonly_t<bool> e_stop_gui;

// used in the gui to decided which file
// to show
extern common::readonly_t<bool> e_show_mine;

extern common::event_t* e_stop_event;

// queues
extern common::queue_st<std::string> e_reader_queue;
extern common::queue_st<common::draw_t> e_writer_queue;

} // namespace client::share
