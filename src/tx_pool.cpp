#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <zmq.h>

#include "defs.hpp"
#include "transaction.hpp"
#include "tx_pool.hpp"

TxPool::TxPool() {}

void TxPool::start(std::string address) {
	auto fp = std::bind(&TxPool::request_handler, this, std::placeholders::_1, std::placeholders::_2);
	
	if(server.start(address, fp) < 0)
		throw std::runtime_error("Server could not bind.");
}

int TxPool::add_transaction(Transaction tx) {
    std::unique_lock<std::mutex> lock(tx_lock);

    display_transaction(tx);
    // TODO: check transaction id

    if(!verify_transaction_signature(tx)) {
        printf("Signature invalid. Rejecting transaction!\n");
        return -1;
    }

    printf("Signature valid, adding transaction...\n");
    transactions.push(tx);

    return 0;
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

void TxPool::request_handler(void* receiver, Message<MessageBuffer> request) {
    if(request.type == POP_TX) {
        auto txs = pop_transactions();
        auto bytes = serialize_message(txs, STATUS_GOOD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
    } else if(request.type == POST_TX) {
        auto tx = deserialize_payload<Transaction>(request.buffer);
        int status = add_transaction(tx);
        auto bytes = serialize_message<NullMessage>({}, (status < 0) ? STATUS_BAD : STATUS_GOOD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
    }  else if(request.type == QUERY_TX_STATUS) {
        auto bytes = serialize_message<Transaction::Status>(Transaction::UNKNOWN, STATUS_GOOD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
    } else {
        throw std::runtime_error("Unknown message type.");
    }
}