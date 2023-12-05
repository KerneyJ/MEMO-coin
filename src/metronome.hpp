#include <array>
#include <condition_variable>
#include <cstdint>
#include <mutex>
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

class Metronome {
    private:
        bool sleeping;
        uint64_t prev_solved_time;
        uint64_t curr_solved_time;
        uint32_t difficulty;
        std::mutex diff_mutex;
        std::mutex block_mutex;
        std::condition_variable block_timer;
        BlockHeader last_block;
        std::string blockchain;
        Server server;
        int active_validators;
        void update_difficulty(bool timed_out);
        void submit_empty_block();
        int submit_block(Block block);
        BlockHeader request_last_block();
        void handle_block(zmq::socket_t &client, MessageBuffer data);
        void get_difficulty(zmq::socket_t &client, MessageBuffer data);
        void register_validator(zmq::socket_t &client, MessageBuffer data);
        void request_handler(zmq::socket_t &client, Message<MessageBuffer> request);
    public:
        Metronome(std::string);
        void start(std::string address);
};