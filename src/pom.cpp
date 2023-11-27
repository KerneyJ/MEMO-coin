#include <cstdlib>
#include <errno.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "wallet.hpp"
#include "config.hpp"
#include "pom.hpp"

static bool check_solution(Blake3Hash result, uint32_t difficulty) {
    int i, j, correct = 0;

    if(difficulty > result.size())
        throw std::runtime_error("Bad difficulty.");

    for(int i = 0; i < difficulty; i++) {
        for(int j = 0; j < 8; ++j)
            if(result[i] >> j & 1){
                return correct >= difficulty;
            } else {
                correct++;
            }
    }

    return correct >= difficulty;
}

bool comphash(Blake3Hash h1, Blake3Hash h2){
    uint8_t* h1buf = (uint8_t*)h1.data();
    uint8_t* h2buf = (uint8_t*)h2.data();
    for(int i = 0; i < 4; i++){
        if(h1buf[i] < h2buf[i])
            return true;
    }
    return false;
}

ProofOfMemory::ProofOfMemory(Wallet wallet){
    this->wallet = wallet;
    this->gen_hashes();
}

void ProofOfMemory::gen_hashes(){
    uint64_t nonce = 0;
    uint8_t* nonce_buf;
    Blake3Hash base, solution, result;
    blake3_hasher hasher;
    srand(time(NULL));
    // initialize base and sol to rand
    for(int i = 0; i < solution.size(); i++)
        base[i] = solution[i] = rand() % 256;

    for(int i = 0; i < this->hashes.max_size(); i++){
        nonce_buf = (uint8_t*)&nonce;

        for(int i = 0; i < 8; i++)
            solution[i] = base[i] + nonce_buf[i];

        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, solution.data(), BLAKE3_OUT_LEN);
        blake3_hasher_update(&hasher, this->wallet.pub_key.data(), BLAKE3_OUT_LEN);
        blake3_hasher_finalize(&hasher, result.data(), BLAKE3_OUT_LEN);

        hashes[nonce] = std::pair<Blake3Hash, Blake3Hash>(result, solution);
        printf("Generating %lu/%lu hashes\r", nonce, this->hashes.max_size());
        nonce++;
    }
    printf("\n");

}


Blake3Hash ProofOfMemory::solve_hash(Blake3Hash prev_hash, uint32_t difficulty) {
    blake3_hasher hasher;
    Blake3Hash h, result;
    // for now search for hash linearly through the array
    for(uint64_t i = 0; i < this->hashes.max_size(); i++){
        if(i % 1000000 == 0)
            printf("Solving hash ...\n");
        h = this->hashes[i].first;
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, prev_hash.data(), BLAKE3_OUT_LEN);
        blake3_hasher_update(&hasher, h.data(), BLAKE3_OUT_LEN);
        blake3_hasher_finalize(&hasher, result.data(), BLAKE3_OUT_LEN);
        if(check_solution(result, difficulty)){
            printf("Solved hash in %lu iteartions.\n", i);
            return result;
        }
    }
    result.fill(0);
    return result;
}