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
        void get_balance(void* receiver, MessageBuffer data);
        void last_block(void* receiver, MessageBuffer data);
        void tx_status(void *receiver, MessageBuffer data);
        void request_handler(void* receiver, Message<MessageBuffer> request);
        void load_genesis();
    public:
        BlockChain();
        void start(std::string address);
};
