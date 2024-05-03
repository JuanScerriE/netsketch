// share
#include "share.hpp"

namespace exporter::share {

threading::thread reader_thread {};
threading::thread writer_thread {};
threading::thread input_thread {};

std::string username {};

bool show_mine { false };

bool run_gui { true };

threading::mutex writer_mutex {};
threading::cond_var writer_cond {};
std::queue<Action> writer_queue {};

// double instance locking state

// NOTE: I am not sure if rwlocks in POSIX favour the writer as identified by
// but I don't think that should be an issue in our implementation

threading::mutex tagged_draw_vector_mutex {};

threading::rwlock rwlock1 {};
threading::rwlock rwlock2 {};

TaggedDrawVector vec1 {};
TaggedDrawVector vec2 {};

} // namespace client::exporter
