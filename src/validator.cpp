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
//Registers at each block so that the statistic is current with the current block.
void register_with_metronome(Wallet wallet) {
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    std::string metronome_address = get_metronome_address();
    zmq_connect(requester, metronome_address.c_str());

    Message<int> response;
    int num_addresses = response.data;
    request_response(requester, REGISTER_VALIDATOR, response);
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return;
}
void Validator::start(std::string address) {
    BlockHeader last_block;

    while(true) {
        // Get next consensus problem
        auto curr_block = request_block_header(last_block);
        auto difficulty = request_difficulty();
        register_with_metronome(this->wallet);
        auto solution = consensus->solve_hash(curr_block.hash, difficulty, curr_block.timestamp + BLOCK_TIME * 1000000);
        
        uint64_t* bytes = (uint64_t*) solution.second.data();
        if(bytes[0] == 0 && bytes[1] == 0 && bytes[2] == 0 && bytes[3] == 0)
            continue;

        auto block = create_block(curr_block, solution.first, solution.second, difficulty);
        
        printf("Submitting block %d.\n", block.header.id);
        int status = submit_block(block);

        if(status < 0) {
            printf("Error submitting block %d!\n", block.header.id);
        }

        last_block = curr_block;
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

BlockHeader Validator::request_block_header(BlockHeader last_block) {
    const int attempts = 12;
    const int timeout = 500;
    
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, blockchain.c_str());

    for(int i = 0; i < attempts; i++) {
        Message<BlockHeader> response;
        request_response(requester, QUERY_LAST_BLOCK, response);
        // printf("if %lu > %lu\n", response.data.id, last_block.id);
        // FIXME there must be a better way than checking if response data is 0, this is a huge security meme
        if(response.data.id > last_block.id || response.data.id == 0) {
            zmq_close(requester);
            return response.data;
        }

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

int Validator::submit_block(Block block) {
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, metronome.c_str());

    Message<NullMessage> response;
    request_response(requester, block, SUBMIT_BLOCK, response);

    zmq_close(requester);
    return response.type == STATUS_GOOD ? 0 : -1;
}
