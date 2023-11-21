#include <array>
#include <cstdint>
#include <signal.h>
#include <zmq.h>

#include "messages.hpp"
#include "thread_pool.hpp"

#pragma once

typedef std::function<void(void*, Message<MessageBuffer>)> msg_func;

class Server {
    private:
        void* context;
        void* router;
        void* dealer;
        ThreadPool* threads;
        volatile sig_atomic_t interrupt;
        static void server_loop(void*, msg_func, volatile sig_atomic_t*);
    public:
        Server();
        ~Server();
        int start(std::string address, msg_func message_handler, bool blocking = true);
};
