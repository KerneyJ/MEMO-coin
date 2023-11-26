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

Block Validator::create_block(BlockHeader prev_block, int difficulty, Blake3Hash solution) {
    auto txs = request_txs();

    // TODO: add reward to txs
    // TODO: variable difficulty

    Block new_block = {
        {
            .hash = solution,
            .prev_hash = prev_block.hash,
            .difficulty = difficulty,
            .timestamp = get_timestamp()
        },
        txs
    };

    return new_block;
}

BlockHeader Validator::request_block_header() {
    Blake3Hash prev_hash, hash;
    hash.fill(255);

    return { hash, prev_hash, 0, 0 };

    ReceiveBuffer res_buf;
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);

    zmq_connect(requester, blockchain.c_str());

    auto req_buf = serialize_message(QUERY_LAST_BLOCK);
    zmq_send(requester, req_buf.data(), req_buf.size(), 0);

    zmq_recv(requester, res_buf.data(), res_buf.size(), 0);
    auto response = deserialize_message<BlockHeader>(res_buf);

    zmq_close(requester);

    return response.buffer;
}

int Validator::request_difficulty() {
    ReceiveBuffer res_buf;
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);

    zmq_connect(requester, metronome.c_str());

    auto req_buf = serialize_message(QUERY_DIFFICULTY);
    zmq_send(requester, req_buf.data(), req_buf.size(), 0);

    zmq_recv(requester, res_buf.data(), res_buf.size(), 0);
    auto response = deserialize_message<int>(res_buf);

    zmq_close(requester);

    return response.buffer;
}

std::array<Transaction, BLOCK_SIZE> Validator::request_txs() {
    return {};

    ReceiveBuffer res_buf;
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);

    zmq_connect(requester, tx_pool.c_str());

    auto req_buf = serialize_message(POP_TX);
    zmq_send(requester, req_buf.data(), req_buf.size(), 0);

    zmq_recv(requester, res_buf.data(), res_buf.size(), 0);
    auto response = deserialize_message<std::array<Transaction, BLOCK_SIZE>>(res_buf);

    zmq_close(requester);

    return response.buffer;
}

int Validator::submit_block(Block block) {
    ReceiveBuffer res_buf;
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);

    zmq_connect(requester, metronome.c_str());

    auto req_buf = serialize_message(block, SUBMIT_BLOCK);
    zmq_send(requester, req_buf.data(), req_buf.size(), 0);

    zmq_recv(requester, res_buf.data(), res_buf.size(), 0);
    auto response = deserialize_message<NullMessage>(res_buf);

    zmq_close(requester);

    return response.type == STATUS_GOOD ? 0 : -1;
}