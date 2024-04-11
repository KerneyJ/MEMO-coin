#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <zmq.hpp>

#include "consensus.hpp"
#include "block.hpp"
#include "defs.hpp"
#include "server.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "defs.hpp"
#include "messages.hpp"

#pragma once
// TODO make function to verify blocks
class Validator {
    private:
        zmq::context_t zmq_ctx;
        std::string blockchain;
        std::string metronome;
        std::string tx_pool;
        IConsensusModel* consensus;
        Wallet wallet;
        Block create_block(BlockHeader bh, HashInput input, Blake3Hash solution, uint32_t difficulty);
        bool submit_block(Block block);
        bool register_with_metronome();
        void request_new_block_header(BlockHeader&);
        uint32_t request_difficulty();
        std::vector<Transaction> request_txs();
    public:
        Validator(std::string, std::string, std::string, IConsensusModel*, Wallet);
        ~Validator();
        void start(std::string);
};
