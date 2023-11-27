#include <stdexcept>
#include <vector>
#include <cassert>
#include <zmq.h>

#include "blockchain.hpp"

BlockChain::BlockChain() {}

void BlockChain::start(std::string address) {
    auto fp = std::bind(&BlockChain::request_handler, this, std::placeholders::_1, std::placeholders::_2);

    if(server.start(address, fp) < 0)
        throw std::runtime_error("server could not bind.");
}

void BlockChain::sync_bal(Block b){
    for(size_t nt = 0; nt < b.transactions.size(); nt++){
        Transaction t = b.transactions[nt];
        auto srcbal = this->ledger.find(t.src);
        auto dstbal = this->ledger.find(t.dst);
        if(srcbal != this->ledger.end())
            this->ledger[src] = srcbal - t.amount;
        else
            ; // this should never happend

        if(dstbal != this->ledger.end())
            this->ledger[dst] = dstbal + t.amount;
        else
            this->ledger.emplace(dst, t.amount);
    }
}

uint32_t BlockChain::get_balance(Ed25519Key pubkey){
    auto bal = this->ledger.find(pubkey);
    if(bal == this->ledger.end())
        return 0;
    return bal;
}

int BlockChain::add_block(Block block){
    // TODO ensure block is valid
    // TODO send the list of transactions in the accepted block the the tx pool
    this->blocks.push_back(block);
    return 0;
}

void BlockChain::request_handler(void* receiver, Message<MessageBuffer> request) {
    if(request.type == QUERY_BAL) {
        uint32_t bal = get_balance();
        auto bytes = serialize_message(bal, STATUS_GOOD);
        zmq_send(receiver, bytes.data(), bytes.size(), 0);
    } else {
        throw std::runtime_error("Unknown message type.");
    }
}
