#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <zmq.h>

#include "defs.hpp"
#include "messages.hpp"
#include "transaction.hpp"
#include "tx_pool.hpp"

TxPool::TxPool(std::string _blockchain) {
    blockchain = _blockchain;
}

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
    tx_queue.push_back(tx);
    submitted_txs.insert({ {tx.src, tx.id}, tx});

    auto bytes = serialize_message(STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::pop_transactions(void* receiver, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);
    std::vector<Transaction> txs;

    // TODO: very inefficient, queue with bulk pop would perform better
    // Only take block size - 1 so there is room for the reward
    for(int i = 0; i < BLOCK_SIZE - 1 && !tx_queue.empty(); i++) {
        txs.push_back(tx_queue.front());

        //remove from queue & submitted list
        tx_queue.erase(tx_queue.begin());
        submitted_txs.erase({txs[i].src, txs[i].id});

        // insert into unconfirmed tx list
        unconfirmed_txs.insert({ {txs[i].src, txs[i].id}, txs[i]});
    }

    auto bytes = serialize_message(txs, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::query_tx_status(void* receiver, MessageBuffer data) {
    auto tx_key = deserialize_payload<std::pair<Ed25519Key, uint64_t>>(data);

    if(submitted_txs.find(tx_key) != submitted_txs.end()) {
        auto bytes = serialize_message(Transaction::SUBMITTED, STATUS_GOOD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
        return;
    }

    if(unconfirmed_txs.find(tx_key) != unconfirmed_txs.end()) {
        auto bytes = serialize_message(Transaction::UNCONFIRMED, STATUS_GOOD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
        return;
    }

    // TODO: delete when blockchain is usable
    // auto bytes = serialize_message(Transaction::UNKNOWN, STATUS_GOOD);
    // zmq_send (receiver, bytes.data(), bytes.size(), 0);
    // return;

    void* requester = zmq_socket(server.get_context(), ZMQ_REQ);
    zmq_connect(requester, blockchain.c_str());

    Message<Transaction::Status> response;
    request_response(requester, tx_key, QUERY_TX_STATUS, response);

    zmq_close(requester);

    auto bytes = serialize_message(response.data, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::request_handler(void* receiver, Message<MessageBuffer> request) {
    switch (request.type) {
        case POP_TX:
            return pop_transactions(receiver, request.data);
        case POST_TX:
            return add_transaction(receiver, request.data);
        case QUERY_TX_STATUS:
            return query_tx_status(receiver, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
