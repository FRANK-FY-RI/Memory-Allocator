# Custom Memory Pool Allocator (C++)

A lightweight fixed-size memory pool allocator implemented in C++.
The allocator uses a free list with boundary tags to manage memory efficiently inside a preallocated heap.
There are 2 available versions with different allocation strategy.

This project was built as a low-level systems programming exercise to explore how dynamic memory allocation works internally and to benchmark it against the standard new/delete allocator.

## Features

* No Internal Fragmentation
* Size of the Fixed Memory Pool is adjustable by the user
* First-fit and Closest-fit allocation strategy available
* Doubly linked free list
* Coalescing of adjacent free blocks
* Boundary tags for `O(1)` block merging
* Benchmark comparison against new

## Design Overview

The allocator manages a contiguous block of memory of size `N`.
Memory is divided into chunks containing metadata and usable memory.

```bash
+--------------+---------------+--------------+
| block header | usable memory | block footer |
+--------------+---------------+--------------+
  (contents):-                    (contents):-
 size of chunk                   size of chunk
 bool  is it free
 pointer to next
 pointer to prev

```
Free chunks are stored in a doubly linked free list.

## Usage

```bash
#include "memory.h"

int main() {
    memory_allocator<1024> alloc;

    int* arr = (int*)alloc.allocate(sizeof(int) * 10);

    for(int i = 0; i < 10; i++) {
        arr[i] = i;
    }
    alloc.deallocate(arr);
}

```
## Benchmark

Benchmark comparing with `new`:

Note: The benchmark result is produced using `-O3` compiler flag

* First Fit without Coalescing:
Note: This benchmark ran for only 2 iterations unlike the rest of all which ran 20 times becuase it was too slow.
```bash
Time My_alloc:               39604073
Time new:                    4983
```

* First Fit with only Forward Coalescing:
```bash
Time My_alloc:               74147
Time new:                    48701
```

* First Fit with Forward and Backward Coalescing:
```bash
Time My_alloc:               108241
Time new:                    50068
```

* Closest Fit with Forward and Backward Coalescing:
```bash
Time My_alloc:               372133
Time new:                    48532
```

## Limitations

* Fixed memory pool size
* Not thread-safe
* Prone to External fragmentation
* Performance depends on free list size

## Future Improvements

Possible Improvements include:

* Next-fit allocation strategy
* Segregated free lists
* Thread-safe allocator
* Slab allocator variant
* Page Alignment and performance tuning

## License

MIT License
