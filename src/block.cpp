#include "block.hpp"
#include "keys.hpp"
#include <cstring>

void display_block_header(BlockHeader header) {
    printf("Block header:\n");
    printf("\thash: %s\n", base58_encode_key(header.hash).c_str());
    printf("\tprev_hash: %s\n", base58_encode_key(header.prev_hash).c_str());
    printf("\tdifficulty: %d\n", header.difficulty);
    printf("\ttimestamp: %lu\n", header.timestamp);
    printf("\tid: %lu\n", header.id);
}

void display_block(Block block) {
    display_block_header(block.header);

    for(auto tx : block.transactions)
        display_transaction(tx);
}


int cmp_b3hash(Blake3Hash h1, Blake3Hash h2) {
    uint8_t h1a[BLAKE3_OUT_LEN] = {0};
    uint8_t h2a[BLAKE3_OUT_LEN] = {0};
    std::copy(std::begin(h1), std::end(h1), std::begin(h1a));
    std::copy(std::begin(h2), std::end(h2), std::begin(h2a));
    return std::memcmp(h1a, h2a, BLAKE3_OUT_LEN);
}
