#include <vector>
#include <cstdint>
#include <uuid/uuid.h>

#include "../external/blake3/blake3.h"
#include "defs.hpp"
#include "consensus.hpp"
#include "wallet.hpp"

#pragma once

class ProofOfMemory : public IConsensusModel {
    private:
        Wallet wallet;
        UUID fingerprint;
        std::vector<Blake3Hash> hashes;
    public:
        ProofOfMemory(Wallet wallet, UUID fingerprint);
        std::pair<HashInput, Blake3Hash> solve_hash(Blake3Hash, uint32_t, uint64_t = UINT64_MAX);
        bool verify_solution(HashInput, Blake3Hash, Blake3Hash, uint32_t);
        void gen_hashes(uint32_t memory);
};
