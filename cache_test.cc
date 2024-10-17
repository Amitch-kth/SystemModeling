#include <iostream>
#include <cassert>  
#include "cache.h"  

using namespace std;


void initialize_memory(Memory& memory) {
    for (size_t i = 0; i < memory.memory.size(); ++i) {
        memory.memory[i] = static_cast<uint8_t>(i & 0xFF);  
    }
}

// Function to test cache write and read operations
void test_cache_read_write(dm_cache& cache) {
    cout << "Running Cache Read/Write Test..." << endl;

    // Perform write operations
    cache.write(0x1000, 0xDEADBEEF);  
    cache.write(0x1100, 0xCAFEBABE);

    // Perform read operations and check the values
    uint32_t result1 = cache.read(0x1000);
    uint32_t result2 = cache.read(0x1100);

    // Validate the results using assertions
    assert(result1 == 0xDEADBEEF);
    assert(result2 == 0xCAFEBABE);

    cout << "Cache Read/Write Test Passed." << endl;
}

// Function to test cache eviction and dirty line handling
void test_cache_eviction(dm_cache& cache) {
    cout << "Running Cache Eviction Test..." << endl;

    // Write to the first cache line
    cache.write(0x1012, 0x12345678);

    // Now write to another address that will replace this cache line
    cache.write(0x2012, 0x87654321); 

    // Read back the first address,
    uint32_t result = cache.read(0x1012);

    //
    assert(result == 0x12345678);

    cout << "Cache Eviction Test Passed." << endl;
}

// Function to test invalidation and cleaning
void test_cache_invalidate_clean(dm_cache& cache) {
    cout << "Running Cache Invalidate and Clean Test..." << endl;

    // Write some values to the cache
    cache.write(0x45678, 0x11111111);
    cache.write(0x45690, 0x22222222);

    // Invalidate the cache range
    cache.invalidate_range(0x45678, 64);  

    uint32_t result = cache.read(0x45678);

    // Assert the result fetched from memory should be same initial memory pattern
    assert((result & 0xFF) == 0x78);

    // Write a value and then clean the range
    cache.write(0x45678, 0x33333333);
    cache.clean_range(0x45678, 32);
    
    cout << "Cache Invalidate and Clean Test Passed." << endl;
}

// Main testbench function
int main() {
    // Create memory of 1MB and initialize it with a test pattern
    Memory memory(1024 * 1024);
    initialize_memory(memory);

    // Create cache and link it to the memory
    dm_cache cache(memory);

    // Run various tests
    test_cache_read_write(cache);
    test_cache_eviction(cache);
    test_cache_invalidate_clean(cache);

    return 0;
}
