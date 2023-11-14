#include <cstdint>

#include "defs.hpp"

#pragma once

struct Transaction {
    Ed25519Key src;
    Ed25519Key dest;
    Ed25519Signature signature;
    uint64_t id;
    uint32_t amount;
    uint64_t timestamp;
};