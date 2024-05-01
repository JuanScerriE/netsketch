// share
#include "share.hpp"

namespace server::share {

threading::mutex users_mutex {};
std::unordered_set<std::string> users {};

threading::mutex threads_mutex {};
std::list<threading::thread> threads {};
threading::thread updater_thread {};

threading::mutex connections_mutex {};
std::unordered_map<int, IPv4Socket> connections {};

threading::mutex timers_mutex {};
std::list<std::unique_ptr<TimerData>> timers;

threading::mutex update_mutex {};
threading::cond_var update_cond {};
TaggedDrawVector tagged_draw_vector {};
std::queue<Payload> payload_queue {};

float time_out { 10 };

} // namespace server::share
