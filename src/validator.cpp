#include <array>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <ratio>
#include <stdexcept>
#include <thread>
#include <vector>
#include <string>
#include <zmq.h>

#include "block.hpp"
#include "consensus.hpp"
#include "defs.hpp"
#include "messages.hpp"
#include "pow.hpp"
#include "transaction.hpp"
#include "validator.hpp"
#include "wallet.hpp"
#include "utils.hpp"
#include "config.hpp"

Validator::Validator(std::string _blockchain, std::string _metronome, 
    std::string _tx_pool, IConsensusModel* _consensus, Wallet _wallet) 
{
    blockchain = _blockchain;
    metronome = _metronome;
    tx_pool = _tx_pool;
    consensus = _consensus;
    wallet = _wallet;

    zmq_ctx = zmq_ctx_new();
}

Validator::~Validator() {
    zmq_ctx_destroy(zmq_ctx);
    delete consensus;
}

//So that the metronome can track the number of active validators in the network. Reported by monitor.
bool Validator::register_with_metronome() {
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, metronome.c_str());

    Message<int> response;
    request_response(requester, REGISTER_VALIDATOR, response);

    zmq_close(requester);
    return response.type == STATUS_GOOD;
}

void Validator::start(std::string address) {
    BlockHeader curr_block { .id = UINT32_MAX };

    register_with_metronome();

    while(true) {
        // Get next consensus problem
        request_new_block_header(curr_block);
        auto difficulty = request_difficulty();

        // Solve consensus problem
        auto [input, solution] = consensus->solve_hash(curr_block.hash, difficulty, curr_block.timestamp + BLOCK_TIME * 1000000);
        
        if(is_null_hash(solution))
            continue;

        auto block = create_block(curr_block, input, solution, difficulty);

        printf("Submitting block [%d].\n", block.header.id);
        if(!submit_block(block))
            printf("Error submitting block [%d]!\n", block.header.id);
    }
}

Block Validator::create_block(BlockHeader prev_block, HashInput input, Blake3Hash solution, uint32_t difficulty) {
    auto txs = request_txs();

    // Add reward for validator, last slot is left empty by TxPool
    Transaction reward = create_reward_transaction(wallet);
    txs.push_back(reward);

    Block new_block = {
        {
            .input = input,
            .hash = solution,
            .prev_hash = prev_block.hash,
            .difficulty = difficulty,
            .timestamp = get_timestamp(),
            .id = prev_block.id + 1,
        },
        txs
    };

    return new_block;
}

void Validator::request_new_block_header(BlockHeader &curr_block) {
    const int timeout = 500;
    
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, blockchain.c_str());

    while(true) {
        Message<BlockHeader> response;
        request_response(requester, QUERY_LAST_BLOCK, response);
        
        if(response.data.id > curr_block.id || curr_block.id == UINT32_MAX) {
            curr_block = response.data;
            zmq_close(requester);
            return;
        }
        printf("Out of date block, %d <= %d.\n", response.data.id, curr_block.id);

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }

    zmq_close(requester);
    throw std::runtime_error("Could not get new blocks.");
}

uint32_t Validator::request_difficulty() {
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, metronome.c_str());

    Message<uint32_t> response;
    request_response(requester, QUERY_DIFFICULTY, response);

    zmq_close(requester);
    return response.data;
}

std::vector<Transaction> Validator::request_txs() {
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, tx_pool.c_str());

    Message<std::vector<Transaction>> response;
    request_response(requester, POP_TX, response);

    zmq_close(requester);
    return response.data;
}

bool Validator::submit_block(Block block) {
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, metronome.c_str());

    Message<NullMessage> response;
    request_response(requester, block, SUBMIT_BLOCK, response);

    zmq_close(requester);
    return response.type == STATUS_GOOD ? 0 : -1;
}
