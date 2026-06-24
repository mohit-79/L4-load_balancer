#pragma once
#include <atomic>
#include <vector>

extern std::vector<std::atomic<uint64_t>> backend_hits;
extern std::atomic<uint64_t> total_requests;

extern std::atomic<uint64_t> rr_counter;
extern std::vector<std::atomic<int>>
backend_active;