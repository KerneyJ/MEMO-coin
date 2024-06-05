#include "block.hpp"
#include "keys.hpp"
#include "transaction.hpp"
#include "utils.hpp"
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

bool verify_transactions(Block b){
    /* iteratively verify each transaction signature in the tx vector*/
    for(Transaction tx : b.transactions){
        std::string src_key = base58_encode_key(tx.src);
        if(src_key == std::string("11111111111111111111111111111169") || src_key == std::string("11111111111111111111111111111111"))
            continue;
        if(!verify_transaction_signature(tx))
            return false;
    }
    return true;
}

bool verify_solution_pom(HashInput input, Blake3Hash curr_hash, Blake3Hash prev_hash, uint32_t difficulty) {
    Blake3Hash target, result;
    blake3_hasher hasher;

    // Get target hash to match prefix against
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, prev_hash.data(), prev_hash.size());
    blake3_hasher_finalize(&hasher, target.data(), target.size());

    // Check hash input result
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, &input, sizeof(HashInput));
    blake3_hasher_finalize(&hasher, result.data(), result.size());

    return check_prefix(result, target, difficulty);
}

bool verify_solution_pow(HashInput input, Blake3Hash curr_hash, Blake3Hash prev_hash, uint32_t difficulty) {
    Blake3Hash result;
    blake3_hasher hasher;

    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, prev_hash.data(), prev_hash.size());
    blake3_hasher_update(&hasher, curr_hash.data(), curr_hash.size());
    blake3_hasher_finalize(&hasher, result.data(), result.size());

    return check_leading_zeros(result, difficulty);
}

/* TODO right now using b.header.difficulty, anyone can set arbtitrary difficulty so maybe change this? */
bool verify_block(Block b, std::string consensus_type){
    if(b.transactions.empty())
        return true; /* empty blocks get a pass */
    bool vfsl, vftx = verify_transactions(b);
    if(consensus_type == "pow")
        vfsl = verify_solution_pow(b.header.input, b.header.hash, b.header.prev_hash, b.header.difficulty);
    else if(consensus_type == "pom")
        vfsl = verify_solution_pom(b.header.input, b.header.hash, b.header.prev_hash, b.header.difficulty);
    else{
        printf("[ERROR] Invalid consensus type, can't verify block\n");
        return false;
    }
#ifdef DEBUG
    printf("[DEBUG] verify transactions: %b; verify solution: %b;\n", vftx, vfsl);
#endif
    return vftx && vfsl;
}
