#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "consensus.hpp"
#include "defs.hpp"
#include "wallet.hpp"
#include "config.hpp"
#include "pom.hpp"

static bool check_prefix(Blake3Hash result, Blake3Hash target, uint32_t difficulty) {
    int i, j, correct = 0;

    if(difficulty > result.size() * 8)
        throw std::runtime_error("Bad difficulty.");

    for(i = 0; i < result.size(); i++) {
        for(j = 0; j < 8; j++) {
            if((result[i] >> j & 1) == (target[i] >> j & 1)){
                correct++;
            } else {
                return correct >= difficulty;
            }
        }
    }

    return correct >= difficulty;
}

ProofOfMemory::ProofOfMemory(Wallet wallet, UUID fingerprint){
    this->fingerprint = fingerprint;
    this->wallet = wallet;
}

void ProofOfMemory::gen_hashes(uint32_t memory){
    HashInput input;
    Blake3Hash result;
    blake3_hasher hasher;
    
    input.fingerprint = fingerprint;
    input.public_key = wallet.pub_key;
    input.nonce = 0;

    int num_hashes = memory / sizeof(Blake3Hash);
    hashes.resize(num_hashes);

    for(int i = 0; i < hashes.size() ; i++) {
        if(i % 123)
            printf("Generating %d/%lu hashes...\r", i+1, hashes.size());

        input.nonce = i;

        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, &input, sizeof(HashInput));
        blake3_hasher_finalize(&hasher, result.data(), result.size());

        hashes[i] = result;
    }
    printf("Generating %lu/%lu hashes...\r", hashes.size(), hashes.size());
    printf("\n");

    // TODO: sort & binary search
    // printf("Sorting %lu hashes...\n", hashes.size());
    // std::sort(hashes.begin(), hashes.end());
    // printf("Finished sorting!\n");
}

std::pair<HashInput, Blake3Hash> ProofOfMemory::solve_hash(Blake3Hash prev_hash, uint32_t difficulty, uint64_t) {
    HashInput input;
    Blake3Hash target;
    blake3_hasher hasher;

    input.fingerprint = fingerprint;
    input.public_key = wallet.pub_key;

    // Get target hash to match prefix against
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, prev_hash.data(), prev_hash.size());
    blake3_hasher_finalize(&hasher, target.data(), target.size());

    for(int i = 0; i < hashes.size(); i++){
        if(check_prefix(hashes[i], target, difficulty)){
            printf("Found solution at index %d.\n", i);
            input.nonce = i;
            return { input, hashes[i] };
        }
    }

    printf("No solution found in %lu hashes.\n", hashes.size());

    input.fingerprint.fill(0);
    input.public_key.fill(0);
    input.nonce = 0;
    target.fill(0);
    return { input, target };
}

bool ProofOfMemory::verify_solution(HashInput input, Blake3Hash curr_hash, Blake3Hash prev_hash, uint32_t difficulty) {
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
