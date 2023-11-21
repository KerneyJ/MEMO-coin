#include <stdexcept>
#include <vector>
#include <zmq.h>

#include "blockchain.hpp"

BlockChain::BlockChain(){
    std::string address = "tcp://*:10004";
    auto fp = std::bind(&BlockChain::request_handler, this, std::placeholders::_1, std::placeholders::_2);

    if(server.start(address, fp) < 0)
        throw std::runtime_error("server could not bind.");
}

uint32_t BlockChain::get_balance(){
    return 50; // return something
}

int BlockChain::add_block(Block block){
    return 0;
}

void BlockChain::request_handler(void* receiver, Message<MessageBuffer> request) {
    if(request.type == GET_BAL) {
        uint32_t bal = get_balance();
        auto bytes = serialize_message(bal, STATUS_GOOD);
        zmq_send(receiver, bytes.data(), bytes.size(), 0);
    } else {
        throw std::runtime_error("Unknown message type.");
    }
}
