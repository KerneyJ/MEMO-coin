#include <array>
#include <cstdint>

#include "blake3/blake3.h"

#pragma once

#define BLOCK_SIZE 100

typedef std::array<uint8_t, BLAKE3_OUT_LEN> Blake3Hash;
typedef std::array<uint8_t, 32> SHA256Hash;