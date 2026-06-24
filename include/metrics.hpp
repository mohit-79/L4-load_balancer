#pragma once
#include <atomic>

extern std::atomic<uint64_t> total_requests;
extern std::atomic<uint64_t> backend1_hits;
extern std::atomic<uint64_t> backend2_hits;
extern std::atomic<uint64_t> rr_counter;
extern std::atomic<int> backend1_active;
extern std::atomic<int> backend2_active;