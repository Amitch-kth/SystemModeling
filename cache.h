#include <iostream>
#include <vector>
#include <cstring>

       
  
#define BUS_WORD_SIZE 8       
#define TAG_SIZE 20
#define INDEX_SIZE 7
#define CACHE_LINE_MAX_SIZE 32
#define OFFSET_SIZE 5
#define CACHE_MAX_SIZE 128 
#define ADDRESS_BUS_WIDTH 32
#define MEMORY_LATENCY 10     

using namespace std;

// Cache structure
struct cache_line {
    bool valid = false;
    bool dirty = false;
    unsigned int tag = 0;
    uint8_t data[CACHE_LINE_MAX_SIZE];  // 32 bytes per cache line
};

// Memory interface
class Memory {
public:
    vector<uint8_t> memory;

    Memory(size_t size) {
        try {
            memory.resize(size, 0);
             std::cout << "Memory resized successfully!" << std::endl;
        }
        catch (const std::bad_alloc& e) {
            std::cerr << "Memory Resize failed: " << e.what() << std::endl;
        }
    }

    // Read cache line from memory, one 64-bit (8-byte) word at a time
    void read_line(uint32_t addr, uint8_t* buffer) {
        std::cout << "Reading from Cache" << addr << std::endl;
        memcpy(buffer, &memory[addr], CACHE_LINE_MAX_SIZE);
    }

    // Write cache line back to memory, one 64-bit word at a time
    void write_line(uint32_t addr, uint8_t* buffer) {
        std::cout << "Writing to Cache" << addr << std::endl;
        memcpy(&memory[addr], buffer, CACHE_LINE_MAX_SIZE);
    }
};

// Cache Class
class dm_cache {
private:
    vector<cache_line> cache;
    Memory& memory;
    unsigned int cycle_counter; 

public:
    dm_cache(Memory& mem) : memory(mem), cycle_counter(0) {
        try {
            cache.resize(CACHE_MAX_SIZE);
            std::cout << "Cache resized successfully!" << std::endl;
        }
        catch (const std::bad_alloc& e){
            std::cout << "Cache resized successfully!" << std::endl;
        }
    }

    void split_address(uint32_t address, unsigned int &tag, unsigned int &index, unsigned int &offset) {
        tag = (address >> (INDEX_SIZE + OFFSET_SIZE)) & ((1 << TAG_SIZE) - 1);  
        index = (address >> OFFSET_SIZE) & ((1 << INDEX_SIZE) - 1);             
        offset = address & ((1 << OFFSET_SIZE) - 1);

        cout << "Address split is done for address" << std::hex << address << std::endl;
        cout << "tag " << tag << "index" << index << "offset" << offset << std::endl;
    }



    uint32_t read(uint32_t address) {
        unsigned int tag, index, offset;
        split_address(address, tag, index, offset);

        cout << "reading value of the address" << std::hex << address << std::endl;

        cache_line& line = cache[index];

        if (line.valid && line.tag == tag) {
            cout <<__FUNCTION__ << "cache hit" << std::endl;
            // Cache hit
            uint32_t data;
            memcpy(&data, &line.data[offset], sizeof(uint32_t));
            return data;
        } else {
            // Cache miss
            cout <<__FUNCTION__ << "cache miss" << std::endl;
            if (line.valid && line.dirty) {
                // Write back to memory if the dirty bit is set
                memory.write_line((line.tag << (INDEX_SIZE + OFFSET_SIZE)) | (index << OFFSET_SIZE), line.data);
            }

            // Read new cache line from memory
            memory.read_line(address & ~(CACHE_LINE_MAX_SIZE - 1), line.data);

            // Update cache
            line.tag = tag;
            line.valid = true;
            line.dirty = false;

            uint32_t data;
            memcpy(&data, &line.data[offset], sizeof(uint32_t)); // Fetch 32-bit word

            cout << "Read data " << data << std::endl; 
            return data;
        } 
    }

    
    void write(uint32_t address, uint32_t value) {
        cout << "writing value " << value << " to the address" << address << std::endl;
        unsigned int tag, index, offset;
        split_address(address, tag, index, offset);

        cache_line& line = cache[index];
        
        

        // Check for cache hit
        if (line.valid && line.tag == tag) {
            // Cache hit
            cout <<__FUNCTION__ << "cache hit" << std::endl;
            memcpy(&line.data[offset], &value, sizeof(uint32_t));
            line.dirty = true;
        } else {
            cout <<__FUNCTION__ << "cache miss" << std::endl;
            // Cache miss, load line from memory
            if (line.valid && line.dirty) {
                // Write back to memory if the dirty bit is set
                memory.write_line((line.tag << (INDEX_SIZE + OFFSET_SIZE)) | (index << OFFSET_SIZE), line.data);
            }

            // Read new cache line from memory
            memory.read_line(address & ~(CACHE_LINE_MAX_SIZE - 1), line.data);

            // Update cache
            line.tag = tag;
            line.valid = true;
            line.dirty = false;

            // store the value in cache
            memcpy(&line.data[offset], &value, sizeof(uint32_t));
            line.dirty = true;
        }
    }

    void invalidate_range(uint32_t startAddr, uint32_t size) {
        for (uint32_t addr = startAddr; addr < startAddr + size; addr += CACHE_LINE_MAX_SIZE) {
            unsigned int tag, index, offset;
            split_address(addr, tag, index, offset);
            cache_line& line = cache[index];
            line.valid = false;  
            line.dirty = false;  
        }
    }

    // Clean cache entries in a range
    void clean_range(uint32_t startAddr, uint32_t size) {
        for (uint32_t addr = startAddr; addr < startAddr + size; addr += CACHE_LINE_MAX_SIZE) {
            unsigned int tag, index, offset;
            split_address(addr, tag, index, offset);

            cache_line& line = cache[index];
            if (line.valid && line.dirty) {
                // Write back dirty cache line
                memory.write_line((line.tag << (INDEX_SIZE + OFFSET_SIZE)) | (index << OFFSET_SIZE), line.data);
                line.dirty = false;
            }
            
        }
    }

    // Clean and invalidate cache entries in a range
    void clean_and_invalidate_range(uint32_t startAddr, uint32_t size) {
        for (uint32_t addr = startAddr; addr < startAddr + size; addr += CACHE_LINE_MAX_SIZE) {
            unsigned int tag, index, offset;
            split_address(addr, tag, index, offset);

            cache_line& line = cache[index];
            if (line.valid && line.dirty) {
                // Write back dirty cache line
                memory.write_line((line.tag << (INDEX_SIZE + OFFSET_SIZE)) | (index << OFFSET_SIZE), line.data);
                line.dirty = false;
            }
            // Invalidate the line
            line.valid = false;
              // Simulate a cycle for each clean+invalidate operation
        }
    }
};

#if 0
// Simulated CPU interaction with cache
int main() {
    Memory memory(1024 * 1024);  // Initialize memory of 1MB
    dm_cache cache(memory);

    // Write to cache
    cout << "writing to the address " << std::hex << 0x012345 << " value" << 42 << std::endl;
    cache.write(0x012345, 42);

    cout << "Data at 0x123456: " << cache.read(0x012345) << endl << std::endl;
    

    cout << "writing to the address " << std::hex << 0x012346 << " value" << 84;
    cache.write(0x012346, 84);

    // Read from cache
    
    cout << "Data at 0x12345: " << cache.read(0x12345) << endl;



    cache.invalidate_range(0x12345, 0x100);

}
#endif