// share
#include "share.hpp"

namespace test_client::share {

bool only_drawing { true };
std::string username {};
uint32_t expected_responses {};

threading::thread reader_thread {};
threading::thread writer_thread {};
threading::thread input_thread {};

threading::mutex writer_mutex {};
threading::cond_var writer_cond {};
std::queue<Action> writer_queue {};

TaggedDrawVector tagged_draw_vector {};

} // namespace test_client::share
