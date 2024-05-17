#include <vector>
#include <mutex>

#include "keys.hpp"
#include "server.hpp"
#include "block.hpp"
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#pragma once

class BlockChain {
    // TODO: checkpoint the ledger to disk and in general make persistent
    // TODO: serialize blocks and write to disk(i.e. make persistent) maybe use sparse file? talk to lan
    private:
        Server server;
        ThreadPool *peer_threads;
        std::string config_file;
        std::string txpool_address;
        std::string file_name;
        std::vector<Block> blocks;
        std::vector<std::string> peers;
        std::mutex blockmutex;
        std::mutex writemutex;
        std::unordered_map<Ed25519Key, uint32_t, Ed25519KeyHash> ledger;
        YAML::Node stored_chain;
        void load_genesis();
        void sync_bal(Block b);
        void add_block(zmq::socket_t &client, MessageBuffer data);
        void get_balance(zmq::socket_t &client, MessageBuffer data);
        void last_block(zmq::socket_t &client, MessageBuffer data);
        void tx_status(zmq::socket_t &client, MessageBuffer data);
        void get_num_addr(zmq::socket_t &client, MessageBuffer data);
        void get_total_coins(zmq::socket_t &client, MessageBuffer data);
        void request_handler(zmq::socket_t &client, Message<MessageBuffer> request);
        /* TODO Maybe store blocks as sparse files
         * since they should all be of a maximum size
         * this would allow us to look up blocks in constant time
         * because each block is at a specific offset in the file
         * sparse files are also more compact
         */
        void load_file(std::string file_name);
        void write_block(Block b);
    public:
        BlockChain(std::string txpaddr, std::string config_file);
        void start(std::string address);
};
