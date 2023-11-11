#include "transaction.hpp"
#include "consensus.hpp"
#include "defs.hpp"

struct Block;
struct BlockHeader;

struct BlockHeader {
    Block* prev_block;
    Blake3Hash hash;
    int difficulty;
    uint64_t timestamp;
};

struct Block {
    BlockHeader header;
    std::array<Transaction, BLOCK_SIZE> transactions;
};
