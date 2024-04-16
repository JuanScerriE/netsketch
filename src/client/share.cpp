// share
#include <share.hpp>

namespace client::share {

// NOTE: for these two variables below we
// are not using atomics since
// only one thread writes whilst the others
// only read additionally, since these are
// just primitive types there is not complexity
// in operations (might change these)

// used to stop the gui upon user exit
common::readonly_t<bool> e_stop_gui{false};

// used in the gui to decided which file
// to show
common::readonly_t<bool> e_show_mine{false};

// used to stop the network thread upon user exit
common::event_t* e_stop_event{nullptr};

// queues
common::queue_st<std::string> e_reader_queue{};
common::queue_st<std::string> e_writer_queue{};

}  // namespace client::share
