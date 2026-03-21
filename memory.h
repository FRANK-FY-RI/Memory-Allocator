/*
    Design:
    This memory allocator manages a fixed heap of size N bytes.
    Closest Fit algorithm is used for allocation.

    Advantages:
    1. No Internal Fragmentation.

    Disadvantages:
    1. Less prone to External Fragmentation than First Fit, but in practice is generally slower than First Fit.

    * Every deallocation tries coalesing to prevent external fragmentation.
*/

#ifndef __MEMORY_H
#define __MEMORY_H
#include <sys/mman.h>
#include <unistd.h>
#include <stdexcept>
#include <cassert>

template <size_t N>
class memory_allocator {
    //Structure of the free list node
    struct free_list_node {
        size_t left;
        bool is_free;
        free_list_node *next;
        free_list_node *prev;
    };

    //Footer
    struct footer {
        size_t left;
    };
    
    void *heap_base, *heap_end;
    free_list_node *head;
    static constexpr const size_t node_size = sizeof(free_list_node);
    static constexpr const size_t foot_size = sizeof(footer);

    //Node remove
    void node_remove(free_list_node *ptr) {
        free_list_node *prev = ptr->prev, *next = ptr->next;
        if(prev != nullptr) prev->next = next;
        else head = next;
        if(next != nullptr) next->prev = prev;
    }

    //Coalesce forward
    void coalesce(free_list_node *ptr) {
        //forward
        free_list_node *next_chunk = (free_list_node*)((std::byte*)ptr + node_size + ptr->left + foot_size);
        if((void*)next_chunk<heap_end && next_chunk->is_free) {
            size_t new_left = ptr->left + next_chunk->left + node_size + foot_size;
            ptr->left = new_left;
            footer *foot = (footer*)((std::byte*)ptr + node_size + ptr->left);
            foot->left = ptr->left;
            node_remove(next_chunk);
        }

        //backward
        footer *prev_foot = (footer*)((std::byte*)ptr - foot_size);
        if((void*)prev_foot>=heap_base) {
            free_list_node *prev_chunk = (free_list_node*)((std::byte*)ptr - foot_size - prev_foot->left - node_size);
            if((void*)prev_chunk>=heap_base && prev_chunk->is_free) {
                prev_chunk->left += (ptr->left + node_size + foot_size);
                footer *foot = (footer*)((std::byte*)prev_chunk + node_size + prev_chunk->left);
                foot->left = prev_chunk->left;
                node_remove(ptr);
            }
        }
    }

public: 

    //Constructor
    memory_allocator() { 
        heap_base = mmap(NULL, node_size + N + foot_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if(heap_base == MAP_FAILED) {
            throw std::runtime_error("Segmentation Fault, not enough memory\n");
        }
        heap_end = (std::byte*)heap_base + node_size + N + foot_size;
        head = (free_list_node*)heap_base;
        head->left = N;
        head->next = nullptr;
        head->is_free = true;
        head->prev = nullptr;
        footer *foot = (footer*)((std::byte*)heap_end - foot_size);
        foot->left = N;
    }

    //Memory Allocator function
    void *allocate(int size) {
        assert(size>0);
        size_t req = node_size + size + foot_size;
        void *ret = nullptr;
        free_list_node *ptr = head, *closest = nullptr;
        size_t diff = N;
        while(ptr != nullptr) {
            if(ptr->left >= req) {
                if(diff > (ptr->left-req)) {
                    diff = ptr->left - req;
                    closest = ptr;
                } 
            }
            ptr = ptr->next;
        }
        if(closest == nullptr) return nullptr;
        ptr = closest;
        free_list_node *new_addr = (free_list_node*)((std::byte*)ptr + node_size + ptr->left + foot_size - req);
        ret = (std::byte*)new_addr+node_size;
        new_addr->left = size;
        new_addr->is_free = false;
        new_addr->next = nullptr;
        ptr->left -= req;
        footer *old_foot = (footer*)((std::byte*)new_addr - foot_size);
        old_foot->left = ptr->left;
        footer *foot = (footer*)((std::byte*)new_addr + node_size + size);
        foot->left = size; 
        return ret;
    }

    //Memory Deallocator function
    void deallocate(void *ptr) {
        assert(ptr != nullptr);
        free_list_node *curr_chunk = (free_list_node*)((std::byte*)ptr-node_size);
        curr_chunk->next = head;
        head->prev = curr_chunk;
        curr_chunk->prev = nullptr;
        curr_chunk->is_free = true;
        head = curr_chunk;
        coalesce(curr_chunk);
    }

    int total_free() {
        free_list_node *ptr = head;
        int sum = 0;
        while(ptr != nullptr) {
            sum += ptr->left;
            ptr = ptr->next;
        }
        return sum;
    }

    //Destructor
    ~memory_allocator() {
        if(heap_base) {
            munmap(heap_base, N+node_size + foot_size);
            head = nullptr;
        } 
    }

    //Copy constructor and copy assignment operator
    memory_allocator(const memory_allocator&)=delete;
    memory_allocator& operator=(const memory_allocator&)=delete;
    //Move constructor and move assignment operator
    memory_allocator(memory_allocator&& rhs) noexcept
    : heap_base(rhs.heap_base),
      heap_end(rhs.heap_end),
      head(rhs.head) { 
        rhs.heap_base = rhs.heap_end = rhs.head = nullptr; 
    }
    memory_allocator& operator=(memory_allocator&& rhs) noexcept {
        if(&rhs != this) {
            if(heap_base) {
                munmap(heap_base, N + node_size + foot_size);
            }
            heap_base = rhs.heap_base;
            heap_end = rhs.heap_end;
            head = rhs.head; 
            rhs.heap_base = rhs.heap_end = rhs.head = nullptr; 
        } 
        return *this;
    }
};

#endif
