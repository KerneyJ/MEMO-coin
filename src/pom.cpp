#include <cstdlib>
#include <errno.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "blake3/blake3.h"
#include "pom.hpp"

bool check_solution(Blake3Hash result, int difficulty) {
    int i, j, correct = 0;

    if(difficulty > result.size())
        throw std::runtime_error("Bad difficulty.");

    for(int i = 0; i < difficulty; i++) {
        for(int j = 0; j < 8; ++j)
            if(result[i] >> j & 1){
                return correct >= diffculty;
            } else {
                correct++;
            }
    }

    return correct >= diffculty;
}

bool comphash(Blake3Hash h1, Blake3Hash h2){
    uint64_t* h1buf = (uint64_t*)h1;
    uint64_t* h2buf = (uint64_t*)h2;
    for(int i = 0; i < 4; i++){
        if(h1buf[i] < h2buf[i])
            return true;
    }
    return false;
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
        blake3_hasher_update(&hasher, /*public key*/, BLAKE3_OUT_LEN);
        blake3_hasher_finalize(&hasher, result.data(), BLAKE3_OUT_LEN);

        hashes.push_back(std::pair<Blake3Hash, Blake3Hash>(result, solution));
        nonce++;
    }

}


Blake3Hash ProofOfMemory::solve_hash(Blake3Hash prev_hash, int difficulty) {
    // for now search for hash linearly through the array
    for(int i = 0; i < 
}

int main(void){
    Blake3Hash prev_hash;
    prev_hash.fill(255);

    IConsensusModel* validator = new ProofOfMemory();

    validator.gen_hashes();
    Blake3Hash solution = validator->solve_hash(prev_hash, 26);

    for(int i = 0; i < solution.size(); i++) {
        printf("%02x", solution[i]);
    }
    printf("\n");
    return 0;
}
