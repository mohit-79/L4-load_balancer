#pragma once
#include <atomic>

extern std::atomic<uint64_t> total_requests;
extern std::atomic<uint64_t> backend1_hits;
extern std::atomic<uint64_t> backend2_hits;
extern std::atomic<uint64_t> rr_counter;
