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
#include <tuple>
#include <unistd.h>
#include <uuid/uuid.h>

#include "consensus.hpp"
#include "defs.hpp"
#include "wallet.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "pom.hpp"

static bool check_prefix(const Blake3Hash &result, const Blake3Hash &target, uint32_t difficulty) {
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

static int prefix_binary_search(const std::vector<Blake3Hash> &hashes, const Blake3Hash& target, int difficulty)
{
    int l = 0, r = hashes.size() - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;

        if (check_prefix(hashes[m], target, difficulty))
            return m;

        if (hashes[m] < target)
            l = m + 1;
        else
            r = m - 1;
    }

    return -1;
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
    uint64_t start = get_timestamp();
    for(int i = 0; i < hashes.size() ; i++) {
        /*if(i % 123)
            printf("Generating %d/%lu hashes...\r", i+1, hashes.size());*/
        input.nonce = i;

        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, &input, sizeof(HashInput));
        blake3_hasher_finalize(&hasher, result.data(), result.size());

        hashes[i] = result;
    }
    printf("Generated %lu hashes in %lu milliseconds\n", hashes.size(), (get_timestamp() - start) / 1000);

    printf("Sorting %lu hashes...\n", hashes.size());
    start = get_timestamp();
    std::sort(hashes.begin(), hashes.end());
    printf("Finished sorting in %lu seconds.\n", (get_timestamp() - start)/1000000);
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

    uint64_t start = get_timestamp();
    int idx = prefix_binary_search(hashes, target, difficulty);
    
    if(idx == -1) {
        printf("No solution found in %lu hashes.\n", hashes.size());

        input.fingerprint.fill(0);
        input.public_key.fill(0);
        input.nonce = 0;
        target.fill(0);
        return { input, target };
    }

    printf("Found solution at index %d in %lu microseconds.\n", idx, get_timestamp() - start);

    input.nonce = 69;
    return { input, hashes[idx] };
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

// int main(void) {
//     UUID fingerprint;
//     uint32_t memory;
//     ProofOfMemory* pom;
//     Wallet wallet;
//     blake3_hasher hasher;

//     uuid_generate(fingerprint.data());
//     set_validator_fingerprint(fingerprint);
//     memory = 10000 * 32;

//     pom = new ProofOfMemory(wallet, fingerprint);
//     pom->gen_hashes(memory);

//     Blake3Hash prev_hash, target;

//     for(int i = 0; i < 100; i++) {

//         prev_hash.fill(i);

//         auto solution = pom->solve_hash(prev_hash, 16);

//         blake3_hasher_init(&hasher);
//         blake3_hasher_update(&hasher, prev_hash.data(), prev_hash.size());
//         blake3_hasher_finalize(&hasher, target.data(), target.size());

//         printf("\ttarget: ");
//         for(auto byte : target)
//             printf("%02x", byte);
//         printf("\n");

//         printf("\tsolution: ");
//         for(auto byte : solution.second)
//             printf("%02x", byte);
//         printf("\n");
//     }
// }