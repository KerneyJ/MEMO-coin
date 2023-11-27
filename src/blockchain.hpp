#include <vector>

#include "keys.hpp"
#include "server.hpp"
#include "block.hpp"

#pragma once

class BlockChain {
    private:
        Server server;
        std::vector<Block> blocks;
        std::unordered_map<Ed25519Key, uint32_t, Ed25519KeyHash> ledger;
        void sync_bal(Block b);
        void add_block(void* receiver, MessageBuffer data);
        void get_balance(void* receiver, MessageBuffer request);
        void request_handler(void* receiver, Message<MessageBuffer> request);
    public:
        BlockChain();
        void start(std::string address);
};
