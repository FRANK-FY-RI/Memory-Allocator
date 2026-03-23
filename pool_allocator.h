#ifndef __POOL_ALLOCATOR_H
#define __POOL_ALLOCATOR_H

#include "memory.h"

template <typename T, size_t N>
class pool_alloc {

    inline static memory_allocator<sizeof(T)*N> pool;

public:

    using value_type = T;
    using size_type = size_t;

    template <typename U>
    struct rebind {
        using other = pool_alloc<U, N>;
    };

    //Default Constructor
    pool_alloc() noexcept = default;

    //Copy Constructor
    pool_alloc(const pool_alloc& rhs) noexcept = default;    

    //Copy Assignment Operator
    pool_alloc& operator=(const pool_alloc& rhs) noexcept = default;

    //Move Constructor
    pool_alloc(pool_alloc&& rhs) noexcept = default;

    //Move Assignment Operator
    pool_alloc& operator=(pool_alloc&& rhs) noexcept = default;

    //Rebind Copy Constructor
    template <typename U>
    pool_alloc(const pool_alloc<U, N>& rhs) noexcept {};

    //Allocate
    value_type* allocate(size_type n) {
        return static_cast<value_type*>(pool.allocate(sizeof(value_type)*n));
    }

    //Deallocate
    void deallocate(value_type *ptr, size_type n) noexcept {
        pool.deallocate(static_cast<void*>(ptr));
    }

    //Comparison friend operators
    template <typename U, size_t M>
    friend bool operator==(const pool_alloc<T, N>& lhs, const pool_alloc<U, M>& rhs) {
        return std::is_same_v<value_type, U> && N==M;
    }

    template <typename U, size_t M>
    friend bool operator!=(const pool_alloc<T, N>& lhs, const pool_alloc<U, M>& rhs) {
        return !(lhs==rhs);
    }

};


#endif