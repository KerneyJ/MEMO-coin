#include <array>
#include <cstdint>
#include <uuid/uuid.h>

#include "../external/blake3/blake3.h"
#include "defs.hpp"

#pragma once

struct HashInput {
    UUID fingerprint;       // ignored for PoW
    Ed25519Key public_key;  // ignored for PoW
    uint64_t nonce;
};

class IConsensusModel {
    public:
        virtual std::pair<HashInput, Blake3Hash> solve_hash(Blake3Hash hash, uint32_t difficulty, uint64_t end_time = UINT64_MAX) = 0;
        virtual bool verify_solution(HashInput, Blake3Hash, Blake3Hash, uint32_t) = 0;
        virtual ~IConsensusModel() {};
};