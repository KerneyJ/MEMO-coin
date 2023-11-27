#include <vector>

#include "server.hpp"
#include "block.hpp"

#pragma once

class BlockChain {
    private:
        Server server;
        std::vector<Block> blocks;
        std::unordered_map<Ed25519Key, uint32_t> ledger;
        void sync_bal(Block b);
        uint32_t get_balance(Ed25519Key pubkey);
        int add_block(Block block);
        void request_handler(void* receiver, Message<MessageBuffer> request);
    public:
        BlockChain();
        void start(std::string address);
};
