// share
#include <share.hpp>

namespace server::share {

threading::mutex e_threads_mutex {};

std::list<threading::pthread> e_threads {};

threading::pthread e_updater_thread {};

threading::pthread e_server_thread {};

threading::mutex e_connections_mutex {};

std::unordered_set<int> e_connections {};

threading::mutex e_timers_mutex {};

std::list<std::unique_ptr<timer_data>> e_timers;

bool e_stop_server { false };

common::ts_queue<prot::tagged_command_t> e_command_queue {};

common::ts_draw_list e_draw_list {};

} // namespace server::share
