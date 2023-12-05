#include <array>
#include <cstdint>
#include <signal.h>
#include <zmq.hpp>

#include "messages.hpp"
#include "thread_pool.hpp"

#pragma once

typedef std::function<void(zmq::socket_t&, Message<MessageBuffer>)> msg_func;

class Server {
    private:
        zmq::context_t context;
        zmq::socket_t router;
        zmq::socket_t dealer;
        ThreadPool* threads;
        volatile sig_atomic_t interrupt;
        static void server_loop(zmq::context_t&, msg_func, volatile sig_atomic_t*);
    public:
        Server();
        ~Server();
        int start(std::string address, msg_func message_handler, bool blocking = true);
        zmq::context_t& get_context();
};
