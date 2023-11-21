#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "consensus.hpp"
#include "block.hpp"
#include "defs.hpp"
#include "server.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "defs.hpp"
#include "messages.hpp"

#pragma once

class Validator {
    private:
        std::string blockchain;
        std::string metronome;
        std::string tx_pool;
        IConsensusModel* consensus;
        Wallet wallet;
        Server server;
        Block create_block(BlockHeader bh, Blake3Hash hash);
        int submit_block(Block block);
        BlockHeader request_block_header();
        std::array<Transaction, BLOCK_SIZE> request_txs();
        void request_handler(void* receiver, Message<MessageBuffer> request);
    public:
        Validator(std::string, std::string, std::string, IConsensusModel*, Wallet);
        ~Validator();
        void start(std::string);
};