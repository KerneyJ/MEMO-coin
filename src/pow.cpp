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

#include "blake3/blake3.h"
#include "pow.hpp"

bool check_solution(Blake3Hash result, int difficulty) {
    int i, j, correct = 0;

    if(difficulty > result.size())
        throw std::runtime_error("Bad difficulty.");

    for (int i = 0; i < difficulty; i++) {
        for(int j = 0; j < 8; ++j) {
            if(result[i] >> j & 1) {
                return correct >= difficulty;
            } else {
                correct++;
            }
        }
    }

    return correct >= difficulty;
}

Blake3Hash ProofOfWork::solve_hash(Blake3Hash prev_hash, int difficulty) {
    uint64_t nonce = 0;
    uint8_t* nonce_buf;
    Blake3Hash base, solution, result;
    blake3_hasher hasher;

    srand(time(NULL));
    for (int i = 0; i < solution.size(); ++i) 
        base[i] = solution[i] = rand() % 256;

    while (true) {
        if(nonce % 10000000 == 0) {
            printf("Solving hash...\n");
        }

        nonce_buf = (uint8_t*) &nonce;

        for(int i = 0; i < 8; i++)
            solution[i] = base[i] + nonce_buf[i];

        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, prev_hash.data(), BLAKE3_OUT_LEN);
        blake3_hasher_update(&hasher, solution.data(), BLAKE3_OUT_LEN);
        blake3_hasher_finalize(&hasher, result.data(), BLAKE3_OUT_LEN);

        if(check_solution(result, difficulty)) {
            printf("Solved hash in %lu iterations.\n", nonce);
            return solution;
        }

        nonce++;
    }
}

int main(void) {
    Blake3Hash prev_hash;
    prev_hash.fill(255);

    IConsensusModel* validator = new ProofOfWork();

    Blake3Hash solution = validator->solve_hash(prev_hash, 26);

    for (int i = 0; i < solution.size(); i++) {
        printf("%02x", solution[i]);
    }
    printf("\n");
    return 0;
}
