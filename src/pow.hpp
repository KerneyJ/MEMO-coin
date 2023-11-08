#include <array>
#include <cstdint>

#include "blake3/blake3.h"
#include "wallet.hpp"

#pragma once

typedef std::array<uint8_t, BLAKE3_OUT_LEN> Blake3Hash;

class ConsensusModel {
    public:
        virtual Blake3Hash solve_hash(Blake3Hash hash, int difficulty) = 0;
};

class ProofOfWork : public ConsensusModel {
    private:
        blake3_hasher hasher;
    public:
        Blake3Hash solve_hash(Blake3Hash hash, int difficulty);
};