#include <array>
#include <cstdint>

#include "../external/blake3/blake3.h"
#include "defs.hpp"
#include "consensus.hpp"

#pragma once

class ProofOfWork : public IConsensusModel {
    public:
        std::pair<HashInput, Blake3Hash> solve_hash(Blake3Hash, uint32_t, uint64_t);
        bool verify_solution(HashInput, Blake3Hash, Blake3Hash, uint32_t);
};