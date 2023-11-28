#include <stdexcept>
#include <vector>
#include <cassert>
#include <zmq.h>

#include "blockchain.hpp"
#include "defs.hpp"
#include "messages.hpp"
#include "wallet.hpp"
#include "transaction.hpp"
#include "utils.hpp"

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
        auto dstbal = this->ledger.find(t.dest);
        if(srcbal != this->ledger.end())
            this->ledger[t.src] = srcbal->second - t.amount;
        else
            ; // this should never happend

        if(dstbal != this->ledger.end())
            this->ledger[t.dest] = dstbal->second + t.amount;
        else
            this->ledger.emplace(t.dest, t.amount);
    }
}

void BlockChain::add_block(void* receiver, MessageBuffer data) {
    auto block = deserialize_payload<Block>(data);

    // TODO ensure block is valid
    // TODO send the list of transactions in the accepted block the the tx pool
    this->blocks.push_back(block);

    auto bytes = serialize_message(STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
}

void BlockChain::get_balance(void* receiver, MessageBuffer data) {
    auto pub_key = deserialize_payload<Ed25519Key>(data);
    
    auto entry = this->ledger.find(pub_key);
    uint32_t balance = (entry == this->ledger.end()) ? 0 : entry->second;

    auto bytes = serialize_message(balance, STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
}

void BlockChain::load_genesis(){
    // pubkey AKb6B2s59CSyig1UmuR5xWxVEwK1aRADHMkwkfgFgNby
    Transaction tx;
    tx.src = NULL;
    tx.dest = "AKb6B2s59CSyig1UmuR5xWxVEwK1aRADHMkwkfgFgNby";
    tx.amount = 50000;
    tx.timestamp = get_timestamp();
    tx.signature = ;
    tx.id = 1;

    std::array<Transaction, BLOCK_SIZE> txs;
    txs[0] = tx;

    Block genesis = {
        {
            .input = NULL,
            .hash = NULL,
            .prev_hash = NULL,
            .difficulty = 1,
            .timestamp = get_timestamp(),
            .id = 1,
        },
        txs,
    };
    this->blocks.push_back(genesis);
}

void BlockChain::request_handler(void* receiver, Message<MessageBuffer> request) {
    switch (request.type) {
        case QUERY_BAL:
            return get_balance(receiver, request.data);
        case SUBMIT_BLOCK:
            return add_block(receiver, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
