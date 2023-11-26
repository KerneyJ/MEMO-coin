#include "utils.hpp"
#include <chrono>

uint64_t get_timestamp() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}