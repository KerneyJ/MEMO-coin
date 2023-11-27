#include <array>
#include <cstdint>

#include "../external/blake3/blake3.h"
#include "defs.hpp"
#include "consensus.hpp"

#pragma once

class ProofOfWork : public IConsensusModel {
    public:
        Blake3Hash solve_hash(Blake3Hash hash, uint32_t difficulty);
};