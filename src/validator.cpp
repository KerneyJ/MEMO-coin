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
#include <zmq.hpp>

#include "block.hpp"
#include "consensus.hpp"
#include "defs.hpp"
#include "messages.hpp"
#include "pow.hpp"
#include "transaction.hpp"
#include "validator.hpp"
#include "wallet.hpp"
#include "utils.hpp"

Validator::Validator(std::string _blockchain, std::string _metronome, 
    std::string _tx_pool, IConsensusModel* _consensus, Wallet _wallet) 
{
    blockchain = _blockchain;
    metronome = _metronome;
    tx_pool = _tx_pool;
    consensus = _consensus;
    wallet = _wallet;
}

Validator::~Validator() {
    delete consensus;
}

//So that the metronome can track the number of active validators in the network. Reported by monitor.
bool Validator::register_with_metronome() {
    zmq::socket_t requester(zmq_ctx, ZMQ_REQ);
    requester.connect(metronome);

    send_message(requester, REGISTER_VALIDATOR);
    auto response = recv_message<NullMessage>(requester);

    return response.header.type == STATUS_GOOD;
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

        printf("Submitting block. [id=%d] [num_txs=%lu]\n", block.header.id, block.transactions.size());
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
    
    zmq::socket_t requester(zmq_ctx, ZMQ_REQ);
    requester.connect(blockchain);

    while(true) {
        send_message(requester, QUERY_LAST_BLOCK);
        auto response = recv_message<BlockHeader>(requester);
        
        if(response.data.id > curr_block.id || curr_block.id == UINT32_MAX) {
            curr_block = response.data;
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }

    throw std::runtime_error("Could not get new blocks.");
}

uint32_t Validator::request_difficulty() {
    zmq::socket_t requester(zmq_ctx, ZMQ_REQ);
    requester.connect(metronome);

    send_message(requester, QUERY_DIFFICULTY);
    auto response = recv_message<uint32_t>(requester);

    return response.data;
}

std::vector<Transaction> Validator::request_txs() {
    zmq::socket_t requester(zmq_ctx, ZMQ_REQ);
    requester.connect(tx_pool);

    send_message(requester, POP_TX);
    auto response = recv_message<std::vector<Transaction>>(requester);

    return response.data;
}

bool Validator::submit_block(Block block) {
    zmq::socket_t requester(zmq_ctx, ZMQ_REQ);
    requester.connect(metronome);

    send_message(requester, block, SUBMIT_BLOCK);
    auto response = recv_message<NullMessage>(requester);

    return response.header.type == STATUS_GOOD;
}
