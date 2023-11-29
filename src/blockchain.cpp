#include <stdexcept>
#include <vector>
#include <cassert>
#include <zmq.h>

#include "blockchain.hpp"
#include "transaction.hpp"
#include "defs.hpp"
#include "messages.hpp"
#include "config.hpp"

BlockChain::BlockChain(std::string txpaddr) {
    this->txpool_address = txpaddr;
}

void BlockChain::start(std::string address) {
    this->load_genesis();
    auto fp = std::bind(&BlockChain::request_handler, this, std::placeholders::_1, std::placeholders::_2);

    if(server.start(address, fp) < 0)
        throw std::runtime_error("server could not bind.");
}

void BlockChain::sync_bal(Block b){
    for(size_t nt = 0; nt < b.transactions.size(); nt++){
        Transaction t = b.transactions[nt];
        auto srcbal = this->ledger.find(t.src);
        auto dstbal = this->ledger.find(t.dest);
        if(srcbal != this->ledger.end()){
            this->ledger[t.src] = srcbal->second - t.amount;
        }
        else {// this should never happend
            this->ledger.emplace(t.src, -t.amount);
        }
        if(dstbal != this->ledger.end()){
            this->ledger[t.dest] = dstbal->second + t.amount;
        }
        else{
            this->ledger.emplace(t.dest, t.amount);
        }
    }
}

void BlockChain::add_block(void* receiver, MessageBuffer data) {
    auto block = deserialize_payload<Block>(data);

    // TODO ensure block is valid
    // TODO send the list of transactions in the accepted block the the tx pool
    this->blocks.push_back(block);
    sync_bal(block);
    auto bytes = serialize_message(STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
    printf("Block added, %lu blocks\n", this->blocks.size());

    void* context = server.get_context();
    void* requester = zmq_socket(context, ZMQ_REQ);

    zmq_connect(requester, txpool_address.c_str());
    Message<NullMessage> response;
    request_response(requester, block, CONFIRM_BLOCK, response);

    zmq_close(requester);
}

void BlockChain::get_balance(void* receiver, MessageBuffer data) {
    auto pub_key = deserialize_payload<Ed25519Key>(data);
    auto entry = this->ledger.find(pub_key);
    uint32_t balance = (entry == this->ledger.end()) ? 0 : entry->second;

    auto bytes = serialize_message(balance, STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
}

void BlockChain::last_block(void* receiver, MessageBuffer data){
    Block b = this->blocks.back();
    auto bytes = serialize_message(b, STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
}

void BlockChain::tx_status(void* receiver, MessageBuffer data){
    auto tx_key = deserialize_payload<std::pair<Ed25519Key, uint64_t>>(data);
    for(size_t nb = 0; nb < this->blocks.size(); nb++){
        Block b = this->blocks[nb];
        for(size_t nt = 0; nt < b.transactions.size(); nt++){
            Transaction t = b.transactions[nt];
            if(t.src == tx_key.first && t.id == tx_key.second){
                auto bytes = serialize_message(Transaction::CONFIRMED, STATUS_GOOD);
                zmq_send(receiver, bytes.data(), bytes.size(), 0);
                return;
            }
        }
    }
    auto bytes = serialize_message(Transaction::UNKNOWN, STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
}

void BlockChain::load_genesis(){
    Block genesis = get_genesis_block();
    this->blocks.push_back(genesis);
    printf("Genesis block loaded\n");
}

//Replies to sender with the number of unique addresses on the blockchain,
//which is equal to the number of entries in blockchain.ledger.
void BlockChain::get_num_addr(void* receiver, MessageBuffer data) {
    printf("getting number of blockchain addresses in blockchain.cpp");
    int num_addresses = this->ledger.size();
    auto bytes = serialize_message(num_addresses, STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
    return;
}

void BlockChain::get_total_coins(void* receiver, MessageBuffer data) {

    // Sum variable to accumulate the values
    uint32_t sum = 0;
    // Iterate over the unordered_map
    printf("\n");
    for (const auto& entry : this->ledger) {
        // entry.first is the key (Ed25519Key)
        // entry.second is the value (uint32_t)
        sum += entry.second;
        printf("entry: %d, ", entry.second);
    }
    printf("\nSUM = %d\n", sum);
    fflush(stdout);
    // Now 'sum' contains the sum of all uint32_t values in the unordered_map
    int total_coins = sum;
    auto bytes = serialize_message(total_coins, STATUS_GOOD);
    zmq_send(receiver, bytes.data(), bytes.size(), 0);
    return;
}

void BlockChain::request_handler(void* receiver, Message<MessageBuffer> request) {
    switch (request.type) {
        case QUERY_BAL:
            return get_balance(receiver, request.data);
        case SUBMIT_BLOCK:
            return add_block(receiver, request.data);
        case QUERY_LAST_BLOCK:
            return last_block(receiver, request.data);
        case QUERY_TX_STATUS:
            return tx_status(receiver, request.data);
        case QUERY_NUM_ADDRS:
            return get_num_addr(receiver, request.data);
        case QUERY_COINS:
            return get_total_coins(receiver, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
