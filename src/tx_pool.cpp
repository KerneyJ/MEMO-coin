#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <zmq.h>

#include "defs.hpp"
#include "messages.hpp"
#include "transaction.hpp"
#include "tx_pool.hpp"

TxPool::TxPool() {}

void TxPool::start(std::string address) {
	auto fp = std::bind(&TxPool::request_handler, this, std::placeholders::_1, std::placeholders::_2);
	
	if(server.start(address, fp) < 0)
		throw std::runtime_error("Server could not bind.");
}

void TxPool::add_transaction(void* receiver, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);
    
    auto tx = deserialize_payload<Transaction>(data);

    printf("Received transaction.\n");
    display_transaction(tx);

    if(!verify_transaction_signature(tx)) {
        printf("Signature invalid. Rejecting transaction!\n");

        auto bytes = serialize_message(STATUS_BAD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
        return;
    }

    printf("Signature valid, adding transaction...\n");
    transactions.push(tx);

    auto bytes = serialize_message(STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::pop_transactions(void* receiver, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);
    std::array<Transaction, BLOCK_SIZE> txs;

    // TODO: very inefficient, queue with bulk pop would perform better
    // Only take block size - 1 so there is room for the reward
    for(int i = 0; i < BLOCK_SIZE - 1 && !transactions.empty(); i++) {
        txs[i] = transactions.front();
        transactions.pop();
    }

    // TODO: add unconfirmed queue

    auto bytes = serialize_message(txs, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::query_tx_status(void* receiver, MessageBuffer data) {
    auto bytes = serialize_message(Transaction::UNKNOWN, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::request_handler(void* receiver, Message<MessageBuffer> request) {
    switch (request.type) {
        case POP_TX:
            return pop_transactions(receiver, request.buffer);
        case POST_TX:
            return add_transaction(receiver, request.buffer);
        case QUERY_TX_STATUS:
            return query_tx_status(receiver, request.buffer);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}