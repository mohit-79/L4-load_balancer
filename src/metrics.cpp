#include "metrics.hpp"
#include "backend_pool.h"
#include <iostream>
std::atomic<uint64_t> total_requests{0};
int x= backends.size();
std::vector<std::atomic<uint64_t>> backend_hits(x);
void initBackendhits(){
for(auto& h : backend_hits) h.store(0);

}
std::atomic<uint64_t> rr_counter{0};
std::vector<std::atomic<int>>
backend_active(backends.size());
void initBackendActive(){
    for(auto& x : backend_active)
{
    x.store(0);
}
}
