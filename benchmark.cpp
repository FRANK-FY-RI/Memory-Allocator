#include "memory.h"
#include <iostream>
#include <chrono>
#include <vector>

using namespace std;
using namespace chrono;

class timer
{
    long long &a;
    decltype(steady_clock::now()) start;

public:
    // Constructor
    timer(long long &val) : a(val), start(steady_clock::now()) {}

    // Destructor
    ~timer()
    {
        auto end = steady_clock::now();
        long long duration = duration_cast<milliseconds>(end - start).count();
        a += duration;
    }
};

uint64_t rand_num = 1;
inline void my_rand()
{
    rand_num ^= rand_num << 13;
    rand_num ^= rand_num >> 7;
    rand_num ^= rand_num << 17;
}

int main()
{
    const int allocations = 100'000;
    memory_allocator<200000000> my_alloc;
    long long time_my_alloc = 0, time_new = 0;
    vector<void *> addrs;
    addrs.reserve(allocations);
    for (int iter = 0; iter < 2; iter++)
    {
        // my alloc
        {
            addrs.clear();
            int i = allocations;
            timer t(time_my_alloc);
            while (i)
            {
                my_rand();
                size_t size = rand_num % 100 + 1;
                if (size & 1 && addrs.size() > 0)
                {
                    int idx = rand_num%addrs.size();
                    void *ptr = addrs[idx];
                    addrs[idx] = addrs.back();
                    addrs.pop_back();
                    my_alloc.deallocate(ptr);
                }
                else
                {
                    void *ptr = my_alloc.allocate(size);
                    addrs.push_back(ptr);
                    i--;
                }
            }
            for (auto it : addrs)
            {
                my_alloc.deallocate(it);
            }
        }
        rand_num = 1;

        // operator new
        {
            addrs.clear();
            int i = allocations;
            timer t(time_new);
            while (i)
            {
                my_rand();
                size_t size = rand_num % 100 + 1;
                if (size & 1 && addrs.size() > 0)
                {
                    int idx = rand_num%addrs.size();
                    void *ptr = addrs[idx];
                    addrs[idx] = addrs.back();
                    addrs.pop_back();
                    ::operator delete(ptr);
                }
                else
                {
                    void *ptr = ::operator new(size);
                    addrs.push_back(ptr);
                    i--;
                }
            }
            for (auto it : addrs)
            {
                ::operator delete(it);
            }
        }
    }

    cout << "Time My_alloc:               " << time_my_alloc << '\n';
    cout << "Time new:                    " << time_new << '\n';
    return 0;
}
