#include <array>
#include <cstdint>
#include <chrono>

#include "../external/blake3/blake3.h"

#pragma once

#define BLOCK_SIZE 100
#define BLOCK_TIME 6
#define MIN_DIFFICULTY 25
#define MINER_REWARD 50

typedef std::array<uint8_t, BLAKE3_OUT_LEN> Blake3Hash;
typedef std::array<uint8_t, 32> Ed25519Key;
typedef std::array<uint8_t, 64> Ed25519Signature;
typedef std::array<uint8_t, 16> UUID;