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
extern common::readonly_t<bool> stop_gui;

// used in the gui to decided which draws
// to show
extern common::readonly_t<bool> show_mine;

// log file
extern logging::log_file log_file;

// writer queue
extern common::ts_queue<prot::tagged_command_t>
    writer_queue;

// nickname
extern std::string nickname;

// double instance locking state
// (http://concurrencyfreaks.blogspot.com/2013/11/double-instance-locking.html)

extern threading::mutex writer_mutex;

extern threading::rwlock rwlock1;
extern threading::rwlock rwlock2;

extern prot::tagged_draw_list_t list1;
extern prot::tagged_draw_list_t list2;

} // namespace client::share
