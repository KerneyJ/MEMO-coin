#include <array>
#include <mutex>
#include <queue>
#include <system_error>
#include <zmq.h>
#include <alpaca/alpaca.h>

#include "defs.hpp"
#include "transaction.hpp"
#include "tx_pool.hpp"

TxPool::TxPool() {
}

TxPool::~TxPool() {
}

void TxPool::add_transaction(Transaction tx) {
    std::unique_lock<std::mutex> lock(tx_lock);

    // TODO: check transaction signature
    // if(!verify_transaction(transactions))
    //     send error

    transactions.push(tx);
}

std::array<Transaction, BLOCK_SIZE> TxPool::pop_transactions() {
    std::unique_lock<std::mutex> lock(tx_lock);
    std::array<Transaction, BLOCK_SIZE> response;

    // TODO: very inefficient, queue with bulk pop would perform better
    for(int i = 0; i < BLOCK_SIZE && !transactions.empty(); i++) {
        response[i] = transactions.front();
        transactions.pop();
    }

    return response;
}

void TxPool::request_handler(void* receiver, Message request) {
    std::error_code ec;
    Transaction tx;

	printf("Received message! type=%d", request.type);

    if(request.type == POP_TX) {
        std::vector<uint8_t> bytes;
        auto txs = pop_transactions();
        alpaca::serialize(txs, bytes);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
    } else if(request.type == POST_TX) {
        auto tx = alpaca::deserialize<Transaction>(request.buffer, ec);
        add_transaction(tx);
    } else {
        throw std::runtime_error("Unknown message type.");
    }
}

int main(void) {
    TxPool tx_pool = TxPool();
    tx_pool.start_server();
}