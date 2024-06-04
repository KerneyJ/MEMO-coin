#include "utils.hpp"
#include <stdexcept>
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

bool check_leading_zeros(const Blake3Hash &result, uint32_t difficulty) {
    int i, j, correct = 0;

    if(difficulty > result.size() * 8)
        throw std::runtime_error("Bad difficulty.");

    for(i = 0; i < result.size(); i++) {
        for(j = 0; j < 8; j++) {
            if(result[i] >> j & 1) {
                return correct >= difficulty;
            } else {
                correct++;
            }
        }
    }

    return correct >= difficulty;
}

bool check_prefix(const Blake3Hash &result, const Blake3Hash &target, uint32_t difficulty) {
    int i, j, correct = 0;

    if(difficulty > result.size() * 8)
        throw std::runtime_error("Bad difficulty.");

    for(i = 0; i < result.size(); i++) {
        for(j = 0; j < 8; j++) {
            if((result[i] >> j & 1) == (target[i] >> j & 1)){
                correct++;
            } else {
                return correct >= difficulty;
            }
        }
    }

    return correct >= difficulty;
}
