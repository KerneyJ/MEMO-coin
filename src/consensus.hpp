#include <array>

#include "blake3/blake3.h"
#include "defs.hpp"

#pragma once

class IConsensusModel {
    public:
        virtual Blake3Hash solve_hash(Blake3Hash hash, int difficulty) = 0;
        virtual ~IConsensusModel() {};
};