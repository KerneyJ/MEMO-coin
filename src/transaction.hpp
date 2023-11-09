#include <cstdint>

#include "defs.hpp"

#pragma once

struct Transaction {
    SHA256Hash src;
    SHA256Hash dest;
    SHA256Hash signature;
    uint32_t id;
    uint32_t amount;
};