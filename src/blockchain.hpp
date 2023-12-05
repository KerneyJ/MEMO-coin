#include <vector>
#include <mutex>

#include "keys.hpp"
#include "server.hpp"
#include "block.hpp"

#pragma once

class BlockChain {
    private:
        Server server;
        std::string txpool_address;
        std::vector<Block> blocks;
        std::mutex blockmutex;
        std::unordered_map<Ed25519Key, uint32_t, Ed25519KeyHash> ledger;
        void load_genesis();
        void sync_bal(Block b);
        void add_block(zmq::socket_t &client, MessageBuffer data);
        void get_balance(zmq::socket_t &client, MessageBuffer data);
        void last_block(zmq::socket_t &client, MessageBuffer data);
        void tx_status(zmq::socket_t &client, MessageBuffer data);
        void get_num_addr(zmq::socket_t &client, MessageBuffer data);
        void get_total_coins(zmq::socket_t &client, MessageBuffer data);
        void request_handler(zmq::socket_t &client, Message<MessageBuffer> request);
    public:
        BlockChain(std::string txpaddr);
        void start(std::string address);
};
