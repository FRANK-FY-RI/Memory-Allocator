/*
    Design:
    This memory allocator manages a fixed heap of size N bytes.
    First Fit algorithm is used for allocation.

    Advantages:
    1. Very simple to implement
    2. Fast

    Disadvantages:
    1. Highly prone to External Fragmentation.
*/

#ifndef __MEMORY_H
#define __MEMORY_H
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <cassert>

template <size_t N>
class memory_allocator {
    //Structure of the free list node
    struct free_list_node {
        int left;
        void *addr;
        free_list_node *next;
    };
    
    void *heap_base;
    free_list_node *head;
    static constexpr const int node_size = sizeof(free_list_node);

public: 

    //Constructor
    memory_allocator() { 
        heap_base = mmap(NULL, node_size + N, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if(heap_base == MAP_FAILED) {
            std::cerr<<"Unable to allocate a memory pool of size "<<84+node_size<<" bytes\n";
            throw std::runtime_error("Segmentation Fault");
        }
        head = (free_list_node*)heap_base;
        head->addr = (std::byte*)heap_base + node_size;
        head->left = N;
        head->next = nullptr;
    }

    //Memory Allocator function
    void *allocate(int size) {
        assert(size>0);
        int req = node_size + size;
        //std::cout<<req<<'\n';
        void *ret = nullptr;
        free_list_node *ptr = head;
        while(ptr != nullptr) {
            if(ptr->left >= req) {
                void *new_addr = (std::byte*)ptr->addr + req; 
                free_list_node *new_chunk = (free_list_node*)ptr->addr;
                new_chunk->left = size;
                new_chunk->addr = (std::byte*)ptr->addr + node_size;
                new_chunk->next = nullptr;
                ret = new_chunk->addr;
                ptr->addr = new_addr;
                ptr->left -= req;
                break;
            }
            ptr = ptr->next;
        }
        return ret;
    }

    //Memory Deallocator function
    void deallocate(void *ptr) {
        assert(ptr != nullptr);
        free_list_node *curr_chunk = (free_list_node*)((std::byte*)ptr-node_size);
        curr_chunk->next = head;
        head = curr_chunk;
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
        if(munmap(heap_base, 84+node_size) == -1) {
            std::cerr<<"Unable to free the heap of size "<<84+node_size<<" at "<<heap_base<<'\n';
            exit(1);
        } 
        head = nullptr;
    }

    //Copy constructor and copy assignment operator
    memory_allocator(const memory_allocator&)=delete;
    memory_allocator& operator=(const memory_allocator&)=delete;
    //Move constructor and move assignment operator
    memory_allocator(memory_allocator&&)=delete;
    memory_allocator& operator=(memory_allocator&&)=delete;
};

#endif
