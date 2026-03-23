#include "pool_allocator.h"
#include <vector>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <iomanip>
#include <string>
#include <numeric>

using ll = long long;

const size_t N = 5e6;
const size_t M = 5e4;
const size_t P = 1e6;

static uint32_t rng_state = 123456789;
inline uint32_t fast_rand() {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

static int run_ind = 0;

std::unordered_map<std::string, std::vector<double>> benchv;

class timer {
    std::string s;
    decltype(std::chrono::steady_clock::now()) start;
    public:
    timer (std::string&& str) {
        s = std::move(str);
        start = std::chrono::steady_clock::now();
    }
    ~timer() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start);
        if(!benchv.contains(s)) benchv[s] = std::vector<double>(2);
        benchv[s][run_ind] += static_cast<double>(duration.count());
    }
};


template <typename Vec>
void benchmark() {
    //push_back
    {
        Vec v;
        timer *t = new timer("push back");
        for(size_t i = 0; i<N; i++) {
            v.push_back(i);
        }
        delete t;
    }
    
    //emplace_back
    {
        Vec v;
        timer *t = new timer("emplace_back");
        for(size_t i = 0; i<N; i++) {
            v.emplace_back(i);
        }
        delete t;
    }
    
    //reserve+push_back
    {
        Vec v;
        timer *t = new timer("reserve+push back");
        v.reserve(N);
        for(size_t i = 0; i<N; i++) {
            v.push_back(i);
        }
        delete t;
    }
    
    //sequential read
    {
        Vec v(N);
        std::iota(v.begin(), v.end(), 0);
    
        timer *t = new timer("sequential read");
        volatile ll sum = 0;
        for(size_t i = 0; i<N; i++) {
            sum += v[i];
        }
        delete t;
    }
    
    //random read
    {
        Vec v(N);
        std::iota(v.begin(), v.end(), 0);
    
        timer *t = new timer("random read");
        volatile ll sum = 0;
        for(size_t i = 0; i<N; i++) {
            uint32_t r = fast_rand();
            uint64_t ind = (uint64_t(r)*N) >> 32;
            sum += v[ind];
        }
        delete t;
    }
    
    //sequential write
    {
        Vec v(N);
        timer *t = new timer("sequential write");
        volatile ll sum = 0;
        for(size_t i = 0; i<N; i++) {
            v[i] = i;
            sum += i;
        }
        delete t;
    }
    
    //random write
    {
        Vec v(N);
        timer *t = new timer("random write");
        for(size_t i = 0; i<N; i++) {
            uint32_t r = fast_rand();
            uint64_t ind = (uint64_t(r)*N) >> 32;
            v[ind] = i;
        }
        delete t;
    }
    
    //range-for iteration
    {
        Vec v(N);
        std::iota(v.begin(), v.end(), 0);
        volatile ll sum = 0;
        timer *t = new timer("range-for iteration");
        for(const auto it:v) {
            sum += it;
        }
        delete t;
    }
    
    //front erase stress test
    {
        Vec v(M);
        std::iota(v.begin(), v.end(), 0);
        timer *t = new timer("front erase stress test");
        for(size_t i = 0; i<M; i++) {
            v.erase(v.begin());
        }
        delete t;
    }

    //range erase stress test
    {
        Vec v(P);
        std::iota(v.begin(), v.end(), 0);
        timer *t = new timer("range erase stress test");
        for(int i = 0; i<100; i++) {
            v.erase(v.begin(), v.begin()+M);
        }
        delete t;
    }
    
    //front insert stress test
    {
        Vec v;
        timer *t = new timer("front insert stress test");
        for(size_t i = 0; i<M; i++) {
            v.insert(v.begin(), i);
        }
        delete t;
    }
    
    //pop_back
    {
        Vec v(N);
        std::iota(v.begin(), v.end(), 0);
        timer *t = new timer("pop_back");
        volatile ll sum = 0;
        for(size_t i = 0; i<N; i++) {
            sum += v[v.size()-1];
            v.pop_back();
        }
        delete t;
    }
    
    //clear+refill
    {
        Vec v(N);
        std::iota(v.begin(), v.end(), 0);
        timer *t = new timer("clear+refill");
        v.clear();
        for(size_t i = 0; i<N; i++) {
            v.emplace_back(i);
        }
        delete t;
    }

    //resize
    {
        Vec v(M);
        std::iota(v.begin(), v.end(), 0);
        timer *t = new timer("resize");
        v.resize(N);
        delete t;
    }
    
    //copy contr
    {
        Vec v1(N);
        std::iota(v1.begin(), v1.end(), 0);
        timer *t = new timer("Copy Constructor");
        Vec v2(v1);
        delete t;
    }
    
    //copy assignment
    {
        Vec v1(N);
        std::iota(v1.begin(), v1.end(), 0);
        Vec v2(M);
        std::iota(v2.begin(), v2.end(), 0);
        timer *t = new timer("Copy Assignment");
        v2 = v1;
        delete t;
    }
    
    //move contr
    {
        Vec v1(N);
        std::iota(v1.begin(), v1.end(), 0);
        volatile ll sum = 0;
        timer *t = new timer("Move Constructor");
        for(int i = 0; i<P; i++) { 
            sum += i;
            Vec v2(std::move(v1));
        }
        delete t;
    }
    
    //move assignment
    {
        Vec v1(N);
        std::iota(v1.begin(), v1.end(), 0);
        Vec v2(M);
        std::iota(v2.begin(), v2.end(), 0);
        volatile ll sum = 0;
        timer *t = new timer("Move Assignment");
        for(int i = 0; i<P; i++) {
            sum += i;
            v2 = std::move(v1);
        } 
        delete t;
    }
}

int main() {
    std::ios::sync_with_stdio(0);
    std::cin.tie(nullptr);
    const int iter = 100;
    for(int i = 0; i<iter; i++) {
        run_ind = 0;
        rng_state = 123456789;
        benchmark<std::vector<int, pool_alloc<int, 8*N>>>();
        run_ind = 1;
        rng_state = 123456789;
        benchmark<std::vector<int>>();
    }
    std::cout<<std::setw(55)<<"custom_memory_pool_allocator"<<std::setw(22)<<"std::allocator\n";
    for(const auto &it:benchv) {
        std::cout<<std::left<<std::setw(35)<<it.first;
        double time1 = (1.0*it.second[0])/iter;
        double time2 = (1.0*it.second[1])/iter;
        std::cout<<std::fixed<<std::setprecision(5)
                 <<std::setw(30)<<time1
                 <<std::setw(30)<<time2<<'\n';
    }
    return 0;
}
