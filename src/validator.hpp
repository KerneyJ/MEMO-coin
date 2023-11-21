#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "consensus.hpp"
#include "block.hpp"
#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "defs.hpp"

#pragma once

class Validator {
    private:
        int difficulty;
        std::string blockchain;
        std::string metronome;
        std::string tx_pool;
        IConsensusModel* consensus;
        Wallet wallet;
        Block create_block(BlockHeader bh, Blake3Hash hash);
        int submit_block(Block block);
        BlockHeader request_block_header();
        std::array<Transaction, BLOCK_SIZE> request_txs();
    public:
        Validator(std::string, std::string, std::string, IConsensusModel* consensus_model);
        ~Validator();
        void run();
};