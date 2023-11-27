#include <array>
#include <cstdint>
#include <ctime>
#include <functional>
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

void Validator::start(std::string address) {
    // TODO: error handling & validation
    while(true) {
        auto blockheader = request_block_header();
        auto difficulty = request_difficulty();
        Blake3Hash solution = consensus->solve_hash(blockheader.hash, difficulty);
        auto block = create_block(blockheader, difficulty, solution);
        
        int status = submit_block(block);

        if(status < 0) {
            printf("Error submitting block.\n");
        } else {
            printf("Submitted block successfully!\n");
        }
    }
}

Block Validator::create_block(BlockHeader prev_block, uint32_t difficulty, Blake3Hash solution) {
    auto txs = request_txs();

    // TODO: add reward to txs
    // TODO: variable difficulty

    Block new_block = {
        {
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

BlockHeader Validator::request_block_header() {
    Blake3Hash prev_hash, hash;
    hash.fill(255);

    return { hash, prev_hash, 0, 0 };

    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, blockchain.c_str());

    Message<BlockHeader> response;
    request_response(requester, QUERY_LAST_BLOCK, response);

    zmq_close(requester);
    return response.data;
}

uint32_t Validator::request_difficulty() {
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, metronome.c_str());

    Message<uint32_t> response;
    request_response(requester, QUERY_DIFFICULTY, response);

    zmq_close(requester);
    return response.data;
}

std::array<Transaction, BLOCK_SIZE> Validator::request_txs() {
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, tx_pool.c_str());

    Message<std::array<Transaction, BLOCK_SIZE>> response;
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