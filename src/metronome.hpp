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

#define BLOCK_TIME 6
#define MIN_DIFFICULTY 20

class Metronome {
    private:
        uint64_t prev_solved_time;
        uint64_t curr_solved_time;
        int difficulty;
        std::mutex diff_mutex;
        std::mutex block_mutex;
        std::condition_variable block_timer;
        std::string blockchain;
        Server server;
        void update_difficulty(bool timed_out);
        void submit_empty_block();
        int submit_block(Block block);
        void handle_block(void* receiver, MessageBuffer data);
        void get_difficulty(void* receiver, MessageBuffer data);
        void request_handler(void* receiver, Message<MessageBuffer> request);
    public:
        Metronome(std::string);
        void start(std::string address);
};