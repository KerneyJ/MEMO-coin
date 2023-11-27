#include <array>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <vector>

#include "defs.hpp"
#include "keys.hpp"
#include "server.hpp"
#include "transaction.hpp"

#pragma once

struct TxKeyHash {
    std::size_t operator () (std::pair<Ed25519Key, uint64_t> const &v) const
    {
        Ed25519KeyHash key_hasher;
        std::hash<uint64_t> long_int_hasher;
        return key_hasher(v.first) ^ long_int_hasher(v.second);    
    }
};

// key on public key & id
typedef std::unordered_map<std::pair<Ed25519Key, uint64_t>, Transaction, TxKeyHash> TransactionMap;

class TxPool {
    private:
        std::string blockchain;
        Server server;
        std::deque<Transaction> tx_queue;
        TransactionMap submitted_txs;
        TransactionMap unconfirmed_txs;
        std::mutex tx_lock;
        void add_transaction(void* receiver, MessageBuffer request);
        void pop_transactions(void* receiver, MessageBuffer request);
        void query_tx_status(void* receiver, MessageBuffer request);
        void request_handler(void* receiver, Message<MessageBuffer> request);
    public:
        TxPool(std::string blockchain);
        void start(std::string address);
};