#include <array>
#include <cstdint>
#include <queue>
#include <vector>

#include "defs.hpp"
#include "server.hpp"
#include "transaction.hpp"

#pragma once

class TxPool {
    private:
        Server server;
        std::queue<Transaction> transactions;
        std::mutex tx_lock;
        void add_transaction(Transaction tx);
        std::array<Transaction, BLOCK_SIZE> pop_transactions();
        void request_handler(void* receiver, Message request);
    public:
        TxPool();
        ~TxPool();
        void start_server();
};