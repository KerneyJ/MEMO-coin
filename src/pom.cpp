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
}

Blake3Hash ProofOfMemory::solve_hash(Blake3Hash prev_hash, int difficulty) {
}

int main(void){
    Blake3Hash prev_hash;
    prev_hash.fill(255);

    IConsensusModel* validator = new ProofOfMemory();

    Blake3Hash solution = validator->solve_hash(prev_hash, 26);

    for(int i = 0; i < solution.size(); i++) {
        printf("%02x", solution[i]);
    }
    printf("\n");
    return 0;
}
