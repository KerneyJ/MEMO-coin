#include "transaction.hpp"
#include "consensus.hpp"
#include "defs.hpp"

#pragma once

struct Block;
struct BlockHeader;

struct BlockHeader {
    Blake3Hash hash;
    Blake3Hash prev_hash;
    uint32_t difficulty;
    uint64_t timestamp;
    uint32_t id;
    /*** 
        Additional fields: 
        - hash digest of all block transactions
        - public key of wallet that mined the block
    ***/
};

struct Block {
    BlockHeader header;
    std::array<Transaction, BLOCK_SIZE> transactions;
};
