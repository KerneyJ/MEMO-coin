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
        void add_transaction(void* receiver, MessageBuffer request);
        void pop_transactions(void* receiver, MessageBuffer request);
        void query_tx_status(void* receiver, MessageBuffer request);
        void request_handler(void* receiver, Message<MessageBuffer> request);
    public:
        TxPool();
        void start(std::string address);
};