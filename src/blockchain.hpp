#include <vector>

#include "server.hpp"
#include "block.hpp"

#pragma once

class BlockChain {
    private:
        Server server;
        std::vector<Block> blocks;
        uint32_t get_balance();
        int add_block(Block block);
        void request_handler(void* receiver, Message<MessageBuffer> request);
    public:
        BlockChain();
        void start(std::string address);
};
