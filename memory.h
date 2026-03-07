/*
    Design:
    This memory allocator manages a fixed heap of size N bytes.
    First Fit algorithm is used for allocation.

    Advantages:
    1. Very simple to implement

    Disadvantages:
    1. Highly prone to External Fragmentation.

    * Every deallocation tries forward coalesing to prevent external fragmentation.
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
    };
    
    void *heap_base, *heap_end;
    free_list_node *head;
    static constexpr const size_t node_size = sizeof(free_list_node);

    //Node remove
    void node_remove(free_list_node *ptr) {
        if(ptr == head) {
            head = head->next;
            ptr->next = nullptr;
            return;
        }
        free_list_node *temp = head;
        free_list_node *prev = nullptr;
        while(temp != ptr) {
            prev = temp;
            temp = temp->next;
        }
        prev->next = temp->next;
        ptr->next = nullptr;
    }

    //Coalesce forward
    void coalesce(free_list_node *ptr) {
        while(true) {
            free_list_node *next_chunk = (free_list_node*)((std::byte*)ptr + node_size + ptr->left);
            if((void*)next_chunk >= heap_end) break;
            if(next_chunk->is_free == false) break;
            size_t new_left = ptr->left + next_chunk->left + node_size;
            ptr->left = new_left;
            node_remove(next_chunk);
        }
    }

public: 

    //Constructor
    memory_allocator() { 
        heap_base = mmap(NULL, node_size + N, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if(heap_base == MAP_FAILED) {
            throw std::runtime_error("Segmentation Fault, not enough memory\n");
        }
        heap_end = (std::byte*)heap_base + node_size + N;
        head = (free_list_node*)heap_base;
        head->left = N;
        head->next = nullptr;
        head->is_free = true;
    }

    //Memory Allocator function
    void *allocate(int size) {
        assert(size>0);
        size_t req = node_size + size;
        //std::cout<<req<<'\n';
        void *ret = nullptr;
        free_list_node *ptr = head;
        while(ptr != nullptr) {
            if(ptr->left >= req) {
                free_list_node *new_addr = (free_list_node*)((std::byte*)ptr + node_size + ptr->left - req);
                ret = (std::byte*)new_addr+node_size;
                new_addr->left = size;
                new_addr->is_free = false;
                new_addr->next = nullptr;
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
        munmap(heap_base, N+node_size);
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
