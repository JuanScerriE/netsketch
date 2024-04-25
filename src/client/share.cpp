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
// common::readonly_t<bool> stop_gui { false };

// used in the gui to decided which file
// to show
// common::readonly_t<bool> show_mine { false };

// log file
logging::log_file log_file {};

// queues
// common::ts_queue<prot::TaggedCommand> writer_queue {};

// nickname
std::string nickname {};

// double instance locking state

// NOTE: I am not sure if rwlocks in POSIX
// favour the writer as identified by the writer
// but I don't think that should be an issue in our
// implementation

threading::mutex writer_mutex {};

threading::rwlock rwlock1 {};
threading::rwlock rwlock2 {};

TaggedDrawVector vec1 {};
TaggedDrawVector vec2 {};

} // namespace client::share
