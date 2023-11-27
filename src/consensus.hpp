#include <array>

#include "../external/blake3/blake3.h"
#include "defs.hpp"

#pragma once

class IConsensusModel {
    public:
        virtual Blake3Hash solve_hash(Blake3Hash hash, uint32_t difficulty) = 0;
        virtual ~IConsensusModel() {};
};