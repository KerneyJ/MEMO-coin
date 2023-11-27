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
	Message<MessageBuffer> request;
	ReceiveBuffer bytes;

    void *receiver = zmq_socket (context, ZMQ_REP);
    status = zmq_connect (receiver, "inproc://workers");

	if(status == -1)
		throw std::runtime_error("Could not connect to socket.");

    while (!(*interrupt)) {
        status = zmq_recv (receiver, bytes.data(), bytes.size(), 0);

        if(status == -1)
			throw std::runtime_error("Could not receive message.");

		request = deserialize_message<MessageBuffer>(bytes);
		message_handler(receiver, request);
    }

    zmq_close (receiver);
}

int Server::start(std::string address, msg_func message_handler, bool blocking) {
	int rc;

	printf("Initializing server at %s.\n", address.c_str());

    router = zmq_socket (context, ZMQ_ROUTER);
    rc = zmq_bind (router, address.c_str());

    dealer = zmq_socket (context, ZMQ_DEALER);
    rc |= zmq_bind (dealer, "inproc://workers");
    
    if(rc != 0) {
		printf("Failed to initialize server, shutting down.\n");
		return -1;
	}

	for(int i = 0; i < threads->size(); i++) {
		threads->queue_job([this, message_handler] {
			server_loop(context, message_handler, &interrupt);
		});
	}

    if(blocking) {
        zmq_proxy (router, dealer, NULL);
    } else {
        std::thread([this] {
            zmq_proxy(router, dealer, NULL);
        }).detach();
    }

    return 0;
}

void* Server::get_context() {
    return context;
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