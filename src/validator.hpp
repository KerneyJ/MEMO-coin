#include <array>
#include <cstdint>
#include <string>

#include "consensus.hpp"
#include "block.hpp"
#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"

#pragma once

class Validator {
    private:
        int difficulty;
        std::string blockchain_address;
        std::string tx_pool_address;
        IConsensusModel* consensus;
        Wallet wallet;
        Block create_block(Blake3Hash hash);
        int submit_block(Block block);
        BlockHeader request_block_header();
        std::array<Transaction, BLOCK_SIZE> request_txs();
    public:
        Validator();
        void run();
};