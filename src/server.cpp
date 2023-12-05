#include <cstdint>
#include <functional>
#include <memory>
#include <signal.h>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include <alpaca/alpaca.h>
#include <zmq.hpp>

#include "server.hpp"

void Server::server_loop(zmq::context_t &context, msg_func message_handler, volatile sig_atomic_t *interrupt) {
    std::error_code ec;
	Message<MessageBuffer> request;
	ReceiveBuffer bytes;

    zmq::socket_t receiver(context, ZMQ_REP);
    receiver.connect("inproc://workers");

    while (!(*interrupt)) {
        auto result = receiver.recv(zmq::buffer(bytes));
		request = deserialize_message<MessageBuffer>(bytes);
		message_handler(receiver, request);
    }
}

int Server::start(std::string address, msg_func message_handler, bool blocking) {
	printf("Initializing server at %s.\n", address.c_str());

    router = zmq::socket_t(context, ZMQ_ROUTER);
    router.bind(address);

    dealer = zmq::socket_t(context, ZMQ_DEALER);
    dealer.bind( "inproc://workers");

	for(int i = 0; i < threads->size(); i++) {
		threads->queue_job([this, message_handler] {
			server_loop(context, message_handler, &interrupt);
		});
	}

    if(blocking) {
        zmq::proxy(router, dealer);
    } else {
        std::thread([this] {
            zmq::proxy(router, dealer);
        }).detach();
    }

    return 0;
}

zmq::context_t& Server::get_context() {
    return context;
}

Server::Server() {
    interrupt = 0;
    threads = new ThreadPool();
}

Server::~Server() {
	printf("Shutting down server.\n");
    interrupt = 1;
    delete threads;
}