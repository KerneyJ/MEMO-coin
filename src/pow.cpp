#include <array>
#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../external/blake3/blake3.h"
#include "consensus.hpp"
#include "defs.hpp"
#include "pow.hpp"
#include "utils.hpp"

std::pair<HashInput, Blake3Hash> ProofOfWork::solve_hash(Blake3Hash prev_hash, uint32_t difficulty, uint64_t end_time) {
    uint64_t nonce = 0;
    uint8_t* nonce_buf;
    Blake3Hash base, input, result;
    blake3_hasher hasher;

    srand(time(NULL));
    for (int i = 0; i < input.size(); ++i) 
        base[i] = input[i] = rand() % 256;

    uint64_t start = get_timestamp();
    while (get_timestamp() < end_time) {
        if(nonce % 10000000 == 0) {
            printf("Solving hash...\n");
        }

        nonce_buf = (uint8_t*) &nonce;

        for(int i = 0; i < 8; i++)
            input[i] = base[i] + nonce_buf[i];

        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, prev_hash.data(), prev_hash.size());
        blake3_hasher_update(&hasher, input.data(), input.size());
        blake3_hasher_finalize(&hasher, result.data(), result.size());

        if(check_leading_zeros(result, difficulty)) {
            uint64_t duration = get_timestamp() - start;
            printf("Solved hash after %lu attempts, duration %lu.\n", nonce, duration);
            return { {}, input };
        }

        nonce++;
    }

    printf("Time expired. No solution found in %lu attempts.\n", nonce);

    input.fill(0);
    return { {}, input };
}

bool ProofOfWork::verify_solution(HashInput input, Blake3Hash curr_hash, Blake3Hash prev_hash, uint32_t difficulty) {
    Blake3Hash result;
    blake3_hasher hasher;

    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, prev_hash.data(), prev_hash.size());
    blake3_hasher_update(&hasher, curr_hash.data(), curr_hash.size());
    blake3_hasher_finalize(&hasher, result.data(), result.size());

    return check_leading_zeros(result, difficulty);
}
