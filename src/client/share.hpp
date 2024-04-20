#pragma once

// common
#include <event.hpp>
#include <log_file.hpp>
#include <protocol.hpp>
#include <types.hpp>

// std
#include <string>

// threading
#include <threading.hpp>

namespace client::share {

extern threading::pthread reader_thread;
extern threading::pthread writer_thread;

extern threading::pthread input_thread;

// used to stop the gui
extern common::readonly_t<bool> e_stop_gui;

// used in the gui to decided which draws
// to show
extern common::readonly_t<bool> e_show_mine;

// log file
extern logging::log_file e_log_file;

// writer queue
extern common::ts_queue<prot::tagged_command_t>
    e_writer_queue;

// nickname
extern std::string e_nickname;

// TODO: figure out a better data structure which
// allows for reading with out locking or acquiring a
// mutex
extern common::ts_queue<std::string> e_reader_queue;

extern prot::tagged_draw_list_t* current_list;
extern prot::tagged_draw_list_t lists[2];

} // namespace client::share
