#include "transaction.hpp"
#include "consensus.hpp"
#include "defs.hpp"
#include <vector>

#pragma once

struct Block;
struct BlockHeader;

struct BlockHeader {
    HashInput input;
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
    std::vector<Transaction> transactions;
};

void display_block_header(BlockHeader header);
void display_block(Block header);
int cmp_b3hash(Blake3Hash h1, Blake3Hash h2);
bool verify_block(Block b);
