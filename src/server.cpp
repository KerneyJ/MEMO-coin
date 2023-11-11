#include <functional>
#include <memory>
#include <signal.h>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include <alpaca/alpaca.h>

#include "server.hpp"

void Server::server_loop(void* context, msg_func message_handler, volatile sig_atomic_t *interrupt) {
	int status;
    std::error_code ec;
	Message request;
	std::array<uint8_t, sizeof(Message)> bytes;

    void *receiver = zmq_socket (context, ZMQ_REP);
    status = zmq_connect (receiver, "inproc://workers");

	if(status == -1)
		throw std::runtime_error("Could not connect to socket.");

    while (!(*interrupt)) {
        status = zmq_recv (receiver, bytes.data(), bytes.size(), 0);

        if(status == -1)
			throw std::runtime_error("Could not receive message.");

		request = alpaca::deserialize<Message>(bytes, ec);
		message_handler(receiver, request);
    }

    zmq_close (receiver);
}

int Server::start(msg_func message_handler) {
	int rc;

    router = zmq_socket (context, ZMQ_ROUTER);
    rc = zmq_bind (router, "tcp://*:5555");

    dealer = zmq_socket (context, ZMQ_DEALER);
    rc |= zmq_bind (dealer, "inproc://workers");
    
    if(rc != 0) {
		printf("Failed to initialize server, shutting down.\n");
		return -1;
	}

	printf("Starting worker threads.\n");
	for(int i = 0; i < threads->size(); i++) {
		threads->queue_job([this, message_handler] {
			server_loop(context, message_handler, &interrupt);
		});
	}

	printf("Starting proxy.\n");
    zmq_proxy (router, dealer, NULL);

    return 0;
}

Server::Server() {
    interrupt = 0;
    threads = new ThreadPool();
    context = zmq_ctx_new ();
}

Server::~Server() {
	printf("Shutting down server.\n");

    zmq_close (router);
    zmq_close (dealer);
    zmq_ctx_destroy (context);

    interrupt = 1;
    delete threads;
}