// share
#include "share.hpp"

namespace test_client::share {

bool only_drawing { true };

threading::thread reader_thread {};
threading::thread writer_thread {};
threading::thread input_thread {};

std::string username {};

threading::mutex writer_mutex {};
threading::cond_var writer_cond {};
std::queue<Action> writer_queue {};

uint32_t expected_responses {};

} // namespace test_client::share
