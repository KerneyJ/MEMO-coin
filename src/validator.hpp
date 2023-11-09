#include <array>
#include <cstdint>
#include <string>

#include "consensus.hpp"
#include "block.hpp"
#include "defs.hpp"
#include "transaction.hpp"

#pragma once

class Validator {
    private:
        int difficulty;
        std::string blockchain_address;
        std::string tx_pool_address;
        IConsensusModel* consensus;
    public:
        Block create_block(Blake3Hash hash);
        int submit_block();
        int request_block_header();
        std::array<Transaction, BLOCK_SIZE> request_txs();
};