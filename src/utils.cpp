#include "utils.hpp"
#include <chrono>
#include <cstdio>

#include "defs.hpp"

uint64_t get_timestamp() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool is_null_hash(Blake3Hash hash) {
    uint64_t* bytes = (uint64_t*) hash.data();
    return bytes[0] == 0 && bytes[1] == 0 && bytes[2] == 0 && bytes[3] == 0;
}