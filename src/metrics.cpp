#include "metrics.hpp"

std::atomic<uint64_t> total_requests{0};
std::atomic<uint64_t> backend1_hits{0};
std::atomic<uint64_t> backend2_hits{0};
std::atomic<uint64_t> rr_counter{0};
std::atomic<int> backend1_active{0};
std::atomic<int> backend2_active{0};