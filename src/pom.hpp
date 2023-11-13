#include <array>
#include <cstdint>

#include "blake3/blake3.h"
#include "defs.hpp"
#include "consensus.hpp"

#pragma once

class ProofOfMemory : public IConsensusModel {
    public:
        Blake3Hash solve_hash(Blake3Hash hash, int difficulty);
        void gen_hashes();

    private:
        std::array<std::pair<Blake3Hash, Blake3Hash>, 1<<30 / BLAKE3_OUT_LEN> hashes;
};
