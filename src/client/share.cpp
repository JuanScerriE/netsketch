// share
#include <share.hpp>

namespace client::share {

threading::pthread reader_thread {};
threading::pthread writer_thread {};

threading::pthread input_thread {};

// NOTE: for these two variables below we
// are not using atomics since
// only one thread writes whilst the others
// only read additionally, since these are
// just primitive types there is not complexity
// in operations (might change these)

// used to stop the gui upon user exit
common::readonly_t<bool> e_stop_gui { false };

// used in the gui to decided which file
// to show
common::readonly_t<bool> e_show_mine { false };

// used to stop the network thread upon user exit
common::event_t* e_stop_event { nullptr };

// log file
logging::log_file e_log_file {};

// queues
common::ts_queue<std::string> e_reader_queue {};
common::ts_queue<prot::tagged_command_t> e_writer_queue {};

// nickname
std::string e_nickname {};

prot::tagged_draw_list_t lists[2] { {}, {} };
prot::tagged_draw_list_t* current_list { &lists[0] };

} // namespace client::share
