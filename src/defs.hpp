#include <array>
#include <cstdint>

#include "../external/blake3/blake3.h"

#pragma once

#define BLOCK_SIZE 100
#define WALLET_PATH "./config/wallet.txt"

typedef std::array<uint8_t, BLAKE3_OUT_LEN> Blake3Hash;

typedef std::array<uint8_t, 32> Ed25519Key;
typedef std::array<uint8_t, 64> Ed25519Signature;