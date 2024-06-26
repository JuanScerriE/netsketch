#include "bench.hpp"
#include <chrono>

namespace bench {

threading::thread benchmark_thread {};
threading::mutex benchmark_mutex {};
threading::cond_var benchmark_cond {};
std::queue<std::pair<std::string, std::chrono::microseconds>>
    benchmark_queue {};

bool disable_individual_logs { false };

} // namespace bench
