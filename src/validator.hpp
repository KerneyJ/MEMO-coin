#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "consensus.hpp"
#include "block.hpp"
#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "address_list.hpp"
#include "defs.hpp"

#pragma once

class Validator {
    private:
        int difficulty;
        AddressList blockchain_peers;
        AddressList tx_pools;
        IConsensusModel* consensus;
        Wallet wallet;
        Block create_block(BlockHeader bh, Blake3Hash hash);
        int submit_block(Block block);
        BlockHeader request_block_header();
        std::array<Transaction, BLOCK_SIZE> request_txs();
    public:
        Validator(AddressList blockchain_peers, AddressList tx_pools, IConsensusModel* consensus_model);
        ~Validator();
        void run();
};