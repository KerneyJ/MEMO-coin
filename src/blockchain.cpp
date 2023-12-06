#include <stdexcept>
#include <vector>
#include <mutex>
#include <cassert>
#include <zmq.hpp>

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
#ifdef DEBUG
        display_transaction(t);
#endif
        auto srcbal = this->ledger.find(t.src);
        auto dstbal = this->ledger.find(t.dest);
        if(dstbal != this->ledger.end()){
            this->ledger[t.dest] = dstbal->second + t.amount;
        }
        else{
            this->ledger.emplace(t.dest, t.amount);
        }
        if(base58_encode_key(t.src) == std::string("11111111111111111111111111111111"))
            continue;
        if(srcbal != this->ledger.end()){
            this->ledger[t.src] = srcbal->second - t.amount;
        }
        else {// this should never happend
            this->ledger.emplace(t.src, -t.amount);
        }
    }
}

void BlockChain::add_block(zmq::socket_t &client, MessageBuffer data) {
    auto block = deserialize_payload<Block>(data);

    // TODO ensure block is valid
    const std::lock_guard<std::mutex> lock(blockmutex);
    this->blocks.push_back(block);
    sync_bal(block);
    send_message(client, STATUS_GOOD);
#ifdef BLOCKCHAIN
    printf("Block added. [id=%d] [num_txs=%lu]\n", block.header.id, block.transactions.size());
#endif
    zmq::context_t& context = server.get_context();
    zmq::socket_t requester(context, ZMQ_REQ);

    requester.connect(txpool_address);

    send_message(requester, block, CONFIRM_BLOCK);
    auto response = recv_message<NullMessage>(requester);
}

void BlockChain::get_balance(zmq::socket_t &client, MessageBuffer data) {
    auto pub_key = deserialize_payload<Ed25519Key>(data);
    auto entry = this->ledger.find(pub_key);
    uint32_t balance = (entry == this->ledger.end()) ? 0 : entry->second;
    send_message(client, balance, STATUS_GOOD);
}

void BlockChain::last_block(zmq::socket_t &client, MessageBuffer data){
    Block b = this->blocks.back();
    send_message(client, b.header, STATUS_GOOD);
}

void BlockChain::tx_status(zmq::socket_t &client, MessageBuffer data){
    auto tx_key = deserialize_payload<std::pair<Ed25519Key, uint64_t>>(data);
    for(size_t nb = 0; nb < this->blocks.size(); nb++){
        Block b = this->blocks[nb];
        for(size_t nt = 0; nt < b.transactions.size(); nt++){
            Transaction t = b.transactions[nt];
            if(t.src == tx_key.first && t.id == tx_key.second){
                send_message(client, Transaction::CONFIRMED, STATUS_GOOD);
                return;
            }
        }
    }
    send_message(client, Transaction::UNKNOWN, STATUS_GOOD);
}

void BlockChain::load_genesis(){
    Block genesis = get_genesis_block();
    this->blocks.push_back(genesis);
#ifdef DEBUG
    printf("Genesis block loaded\n");
#endif
}

//Replies to sender with the number of unique addresses on the blockchain,
//which is equal to the number of entries in blockchain.ledger.
void BlockChain::get_num_addr(zmq::socket_t &client, MessageBuffer data) {
#ifdef DEBUG
    printf("getting number of blockchain addresses in blockchain.cpp");
#endif
    int num_addresses = this->ledger.size();
    send_message(client, num_addresses, STATUS_GOOD);
    return;
}

void BlockChain::get_total_coins(zmq::socket_t &client, MessageBuffer data) {

    // Sum variable to accumulate the values
    uint32_t sum = 0;
    // Iterate over the unordered_map
#ifdef DEBUG
    printf("\n");
#endif
    for (const auto& entry : this->ledger) {
        // entry.first is the key (Ed25519Key)
        // entry.second is the value (uint32_t)
        sum += entry.second;
#ifdef DEBUG
        printf("entry: %d, ", entry.second);
#endif
    }
#ifdef DEBUG
    printf("\nSUM = %d\n", sum);
    fflush(stdout);
#endif
    // Now 'sum' contains the sum of all uint32_t values in the unordered_map
    int total_coins = sum;
    send_message(client, total_coins, STATUS_GOOD);
    return;
}

void BlockChain::request_handler(zmq::socket_t &client, Message<MessageBuffer> request) {
    switch (request.header.type) {
        case QUERY_BAL:
            return get_balance(client, request.data);
        case SUBMIT_BLOCK:
            return add_block(client, request.data);
        case QUERY_LAST_BLOCK:
            return last_block(client, request.data);
        case QUERY_TX_STATUS:
            return tx_status(client, request.data);
        case QUERY_NUM_ADDRS:
            return get_num_addr(client, request.data);
        case QUERY_COINS:
            return get_total_coins(client, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
