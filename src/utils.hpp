#include "defs.hpp"
#include <chrono>

uint64_t get_timestamp();
bool is_null_hash(Blake3Hash);
bool check_leading_zeros(const Blake3Hash &result, uint32_t difficulty);
bool check_prefix(const Blake3Hash &result, const Blake3Hash &target, uint32_t difficulty);
