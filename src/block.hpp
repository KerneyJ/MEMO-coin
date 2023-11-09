#include "transaction.hpp"
#include "consensus.hpp"
#include "defs.hpp"

#define BLOCK_SIZE 100

struct Block;
struct BlockHeader;

struct BlockHeader {
    Block* prev_block;
    Blake3Hash hash;
    int difficulty;
};

struct Block {
    BlockHeader header;
    std::array<Transaction, BLOCK_SIZE> transactions;
};
