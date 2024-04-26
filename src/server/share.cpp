// share
#include "share.hpp"

namespace server::share {

std::atomic_bool run { true };

threading::mutex threads_mutex {};

std::list<threading::thread> threads {};

threading::thread updater_thread {};

threading::mutex connections_mutex {};

std::unordered_map<int, IPv4Socket> connections {};

threading::mutex timers_mutex {};

std::list<std::unique_ptr<TimerData>> timers;

bool e_stop_server { false };

threading::mutex update_mutex {};

threading::cond_var update_cond {};

TaggedDrawVector tagged_draw_vector {};

std::queue<Payload> payload_queue {};

logging::log timing_log{};

} // namespace server::share
