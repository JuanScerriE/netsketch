#include "bench.hpp"

namespace bench {

threading::thread benchmark_thread {};
threading::mutex benchmark_mutex {};
threading::cond_var benchmark_cond {};
std::queue<std::pair<std::string, std::chrono::nanoseconds>> benchmark_queue {};

} // namespace bench
