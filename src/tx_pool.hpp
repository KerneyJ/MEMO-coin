#include <array>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "defs.hpp"
#include "keys.hpp"
#include "server.hpp"
#include "transaction.hpp"

#pragma once

// key on public key & id
struct TxHash {
    std::size_t operator () (Transaction const &t) const
    {
        Ed25519KeyHash key_hasher;
        std::hash<uint64_t> long_int_hasher;
        return key_hasher(t.src) ^ long_int_hasher(t.id);
    }
};

typedef std::unordered_set<Transaction, TxHash> TransactionSet;

class TxPool {
    private:
        std::vector<std::string> blockchains;
        Server server;
        std::deque<Transaction> submitted_queue;
        std::deque<Transaction> unconfirmed_queue;
        TransactionSet submitted_set;
        TransactionSet unconfirmed_set;
        std::mutex tx_lock;
        void add_transaction(zmq::socket_t &client, MessageBuffer request);
        void pop_transactions(zmq::socket_t &client, MessageBuffer request);
        void confirm_transactions(zmq::socket_t &client, MessageBuffer request);
        void query_tx_status(zmq::socket_t &client, MessageBuffer request);
        void confirm_block(zmq::socket_t &client, MessageBuffer request);
        void query_tx_count(zmq::socket_t &client, MessageBuffer request);
        void register_blockchain(zmq::socket_t &client, MessageBuffer request);
        void request_handler(zmq::socket_t &client, Message<MessageBuffer> request);
    public:
        TxPool(std::string blockchain);
        void start(std::string address);
};
