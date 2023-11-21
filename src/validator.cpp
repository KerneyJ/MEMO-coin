#include <array>
#include <cstdint>
#include <ctime>
#include <functional>
#include <vector>
#include <string>

#include "consensus.hpp"
#include "defs.hpp"
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

void Validator::start(std::string address) {
	auto fp = std::bind(&Validator::request_handler, this, std::placeholders::_1, std::placeholders::_2);
	
	if(server.start(address, fp, false) < 0)
		throw std::runtime_error("Server could not bind.");

    // TODO: error handling & validation
    while(true) {
        auto problem = request_block_header();
        Blake3Hash solution = consensus->solve_hash(problem.hash, problem.difficulty);
        auto block = create_block(problem, solution);
        submit_block(block);

        printf("Submitted block!\n");
    }
}

BlockHeader Validator::request_block_header() {
    // TODO: make request to blockchain
    Blake3Hash hash, prev_hash;
    hash.fill(255);

    return { nullptr, hash, prev_hash, 25, 0 };
}

Block Validator::create_block(BlockHeader prev_block, Blake3Hash solution) {
    auto txs = request_txs();

    // TODO: add reward to txs
    // TODO: variable difficulty

    Block new_block = {
        {
            nullptr,
            solution,
            prev_block.hash,
            prev_block.difficulty,
            get_timestamp()
        },
        txs
    };

    return new_block;
}

std::array<Transaction, BLOCK_SIZE> Validator::request_txs() {
    // TODO: make request to tx_pool
    std::array<Transaction, BLOCK_SIZE> txs;

    return txs;
}

int Validator::submit_block(Block block) {
    // TODO: implement

    return 0;
}

void Validator::request_handler(void* receiver, Message<MessageBuffer> request) {

}