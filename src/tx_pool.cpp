#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <zmq.h>

#include "block.hpp"
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
    submitted_queue.push_back(tx);
    submitted_set.insert(tx);

    auto bytes = serialize_message(STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::pop_transactions(void* receiver, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);

    // Only take block size - 1 so there is room for the reward
    auto begin = submitted_queue.begin();
    auto end = BLOCK_SIZE - 1 <= submitted_queue.size() ? begin + BLOCK_SIZE - 1 : submitted_queue.end();
    std::vector<Transaction> txs(begin, end);

    // Update queues & sets
    for(auto tx : txs) {
        // remove from queue & submitted list
        submitted_queue.pop_front();
        submitted_set.erase(tx);

        // insert into unconfirmed tx list
        unconfirmed_queue.push_back(tx);
        unconfirmed_set.insert(tx);
    }

    auto bytes = serialize_message(txs, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::confirm_transactions(void* receiver, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);

    auto block = deserialize_payload<Block>(data);
    auto confirmed_set = TransactionSet(block.transactions.begin(), block.transactions.end());

    while(!confirmed_set.empty() && !unconfirmed_queue.empty()) {
        auto tx = unconfirmed_queue.front();
        unconfirmed_queue.pop_front();
        unconfirmed_set.erase(tx);
        
        // if element was not in confirmed set, add it to submitted queue
        if(confirmed_set.erase(tx) == 0) {
            submitted_queue.push_back(tx);
            submitted_set.insert(tx);
        }
    }

    auto bytes = serialize_message(STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void TxPool::query_tx_status(void* receiver, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);

    auto tx_key = deserialize_payload<std::pair<Ed25519Key, uint64_t>>(data);
    Transaction tx = { .src = tx_key.first, .id = tx_key.second };

    if(submitted_set.find(tx) != submitted_set.end()) {
        auto bytes = serialize_message(Transaction::SUBMITTED, STATUS_GOOD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
        return;
    }

    if(unconfirmed_set.find(tx) != unconfirmed_set.end()) {
        auto bytes = serialize_message(Transaction::UNCONFIRMED, STATUS_GOOD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
        return;
    }

    void* requester = zmq_socket(server.get_context(), ZMQ_REQ);
    zmq_connect(requester, blockchain.c_str());

    Message<Transaction::Status> response;
    request_response(requester, tx_key, QUERY_TX_STATUS, response);

    zmq_close(requester);

    auto bytes = serialize_message(response.data, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}


//Count the number of transactions in the transaction pool.
void TxPool::query_tx_count(void* receiver, MessageBuffer data) {
    int tx_count = submitted_queue.size() + unconfirmed_queue.size();

    //Send the data back to the monitor

    auto bytes = serialize_message(tx_count, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
    return;
}


void TxPool::request_handler(void* receiver, Message<MessageBuffer> request) {
    switch (request.type) {
        case POP_TX:
            return pop_transactions(receiver, request.data);
        case POST_TX:
            return add_transaction(receiver, request.data);
        case QUERY_TX_STATUS:
            return query_tx_status(receiver, request.data);
        case QUERY_TX_COUNT: 
            return query_tx_count(receiver, request.data);
        case CONFIRM_BLOCK:
            return confirm_transactions(receiver, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
