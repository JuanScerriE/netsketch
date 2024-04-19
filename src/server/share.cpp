// share
#include <share.hpp>

namespace server::share {

common::event_t* e_stop_event { nullptr };

std::vector<int> e_connections {};

common::queue_st<prot::tagged_command_t> e_command_queue {};

common::ts_draw_list e_draw_list {};

} // namespace server::share
