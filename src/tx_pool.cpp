#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <zmq.hpp>

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

void TxPool::add_transaction(zmq::socket_t &client, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);

    auto tx = deserialize_payload<Transaction>(data);
#ifdef DEBUG
    printf("Received transaction.\n");
    display_transaction(tx);
#endif
    if(!verify_transaction_signature(tx)) { // TODO remove? make the pool light
#ifdef DEBUG
        printf("Signature invalid. Rejecting transaction!\n");
#endif
        send_message(client, STATUS_BAD);
        return;
    }
#ifdef DEBUG
    printf("Signature valid, adding transaction...\n");
#endif
    submitted_queue.push_back(tx);
    submitted_set.insert(tx);

    send_message(client, STATUS_GOOD);
}

void TxPool::pop_transactions(zmq::socket_t &client, MessageBuffer data) {
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

    send_message(client, txs, STATUS_GOOD);
}

void TxPool::confirm_transactions(zmq::socket_t &client, MessageBuffer data) {
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

    send_message(client, STATUS_GOOD);
}

void TxPool::query_tx_status(zmq::socket_t &client, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(tx_lock);

    auto tx_key = deserialize_payload<std::pair<Ed25519Key, uint64_t>>(data);
    Transaction tx = { .src = tx_key.first, .id = tx_key.second };

    if(submitted_set.find(tx) != submitted_set.end()) {
        send_message(client, Transaction::SUBMITTED, STATUS_GOOD);
        return;
    }

    if(unconfirmed_set.find(tx) != unconfirmed_set.end()) {
        send_message(client, Transaction::UNCONFIRMED, STATUS_GOOD);
        return;
    }

    zmq::socket_t requester(server.get_context(), ZMQ_REQ);
    requester.connect(blockchain);

    send_message(requester, tx_key, QUERY_TX_STATUS);
    auto response = recv_message<Transaction::Status>(requester);

    send_message(client, response.data, STATUS_GOOD);
}


//Count the number of transactions in the transaction pool.
void TxPool::query_tx_count(zmq::socket_t &client, MessageBuffer data) {
    int tx_count = submitted_queue.size() + unconfirmed_queue.size();
    send_message(client, tx_count, STATUS_GOOD);
}


void TxPool::request_handler(zmq::socket_t &client, Message<MessageBuffer> request) {
    switch (request.header.type) {
        case POP_TX:
            return pop_transactions(client, request.data);
        case POST_TX:
            return add_transaction(client, request.data);
        case QUERY_TX_STATUS:
            return query_tx_status(client, request.data);
        case QUERY_TX_COUNT: 
            return query_tx_count(client, request.data);
        case CONFIRM_BLOCK:
            return confirm_transactions(client, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
