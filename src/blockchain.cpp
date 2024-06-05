#include <stdexcept>
#include <random>
#include <vector>
#include <mutex>
#include <zmq.hpp>

#include "utils.hpp"
#include "blockchain.hpp"
#include "consensus.hpp"
#include "transaction.hpp"
#include "defs.hpp"
#include "keys.hpp"
#include "messages.hpp"
#include "config.hpp"

BlockChain::BlockChain(std::string txpaddr, std::string metroaddr, std::string consensus_type, bool sync_chain, std::string config_file) {
    this->txpool_address = txpaddr;
    this->metro_address = metroaddr;
    this->consensus_type = consensus_type;
    this->_sync_chain = sync_chain;
    this->config_file = config_file;
    this->wait = true;
    YAML::Node config = YAML::LoadFile(this->config_file);
    this->file_name = config["blockchain"]["file"].as<std::string>();
    if(!this->file_name.empty())
        this->stored_chain = YAML::LoadFile(file_name);
    for(auto iter : config["blockchain"]["peers"])
        this->peers.push_back(iter["address"].as<std::string>());
    this->peer_threads = new ThreadPool(2);
}

void BlockChain::start(std::string address) {
    this->load_genesis();

    // Check if we need to read from a file
    if(!this->file_name.empty()){
        for(YAML::const_iterator it = this->stored_chain.begin(); it != this->stored_chain.end(); ++it){
            std::string block_name = it->first.as<std::string>();
            if(block_name.find("Block") == std::string::npos)
                break; // this is if we can't find Block in the string
#ifdef DEBUG
            // printf("Parsing block %s\n", block_name.c_str());
#endif
            Block b;
            b.header.hash = base58_decode_key(this->stored_chain[block_name]["header"]["hash"].as<std::string>());
            b.header.prev_hash = base58_decode_key(this->stored_chain[block_name]["header"]["prev_hash"].as<std::string>());
            b.header.difficulty = this->stored_chain[block_name]["header"]["difficulty"].as<uint32_t>();
            b.header.timestamp = this->stored_chain[block_name]["header"]["timestamp"].as<uint64_t>();
            b.header.id = this->stored_chain[block_name]["header"]["id"].as<uint32_t>();
            b.header.input.fingerprint = base58_decode_uuid(this->stored_chain[block_name]["header"]["input"]["fingerprint"].as<std::string>());
            b.header.input.public_key = base58_decode_key(this->stored_chain[block_name]["header"]["input"]["publickey"].as<std::string>());
            b.header.input.nonce = this->stored_chain[block_name]["header"]["input"]["nonce"].as<uint64_t>();
            for(auto iter : this->stored_chain[block_name]["transactions"]) {
                Transaction tx;
                tx.src = base58_decode_key(iter["src"].as<std::string>());
                tx.dest = base58_decode_key(iter["dest"].as<std::string>());
                tx.amount = iter["amount"].as<uint32_t>();
                tx.signature = base58_decode_sig(iter["signature"].as<std::string>());
                tx.timestamp = iter["timestamp"].as<uint64_t>();
                tx.id = iter["id"].as<uint32_t>();
                b.transactions.push_back(tx);
#ifdef DEBUG
                // printf("src: %s; dest: %s; amt: %d\n", iter["src"].as<std::string>().c_str(), iter["dest"].as<std::string>().c_str(), tx.amount);
#endif
            }
            this->blocks.push_back(b);
            sync_bal(b);
        }
    }
    else
        this->file_name = "blockchain.yaml";

    auto fp = std::bind(&BlockChain::request_handler, this, std::placeholders::_1, std::placeholders::_2);
    if(server.start(address, fp, false) < 0)
        throw std::runtime_error("server could not bind.");
    if(_sync_chain)
        sync_chain();
    server.proxy_thread.join();
}

void BlockChain::sync_bal(Block b){
    for(size_t nt = 0; nt < b.transactions.size(); nt++){
        Transaction t = b.transactions[nt];
        std::string src_key = base58_encode_key(t.src);
#ifdef DEBUG
        //display_transaction(t);
#endif
        auto srcbal = this->ledger.find(t.src);
        auto dstbal = this->ledger.find(t.dest);
        if(dstbal != this->ledger.end()){
            this->ledger[t.dest] = dstbal->second + t.amount;
        }
        else{
            this->ledger.emplace(t.dest, t.amount);
        }
        if(src_key == std::string("11111111111111111111111111111169") || src_key == std::string("11111111111111111111111111111111"))
            continue;
        if(srcbal != this->ledger.end()){
            this->ledger[t.src] = srcbal->second - t.amount;
        }
        else {// this should never happend
            printf("[ERROR] Negative balance %s\n", base58_encode_key(t.src).c_str());
            // TODO if this happens should probably role back this block
            // this->ledger.emplace(t.src, -t.amount);
        }
    }
}

void BlockChain::write_block(Block b){
    std::lock_guard<std::mutex> lock(this->writemutex);
    std::string block_name = "Block" + std::to_string(b.header.id);

    // Populate node
    this->stored_chain[block_name]["header"]["input"]["fingerprint"] = base58_encode_uuid(b.header.input.fingerprint);
    this->stored_chain[block_name]["header"]["input"]["publickey"] = base58_encode_key(b.header.input.public_key);
    this->stored_chain[block_name]["header"]["input"]["nonce"] = b.header.input.nonce;
    this->stored_chain[block_name]["header"]["hash"] = base58_encode_key(b.header.hash);
    this->stored_chain[block_name]["header"]["prev_hash"] = base58_encode_key(b.header.prev_hash);
    this->stored_chain[block_name]["header"]["difficulty"] = b.header.difficulty;
    this->stored_chain[block_name]["header"]["timestamp"] = b.header.timestamp;
    this->stored_chain[block_name]["header"]["id"] = b.header.id;
    std::vector<std::map<std::string, std::string>> transactions;
    for(auto iter : b.transactions){
        std::map<std::string, std::string> tx;
        tx["src"] = base58_encode_key(iter.src);
        tx["dest"] = base58_encode_key(iter.dest);
        tx["signature"] = base58_encode_sig(iter.signature);
        tx["id"] = std::to_string(iter.id);
        tx["amount"] = std::to_string(iter.amount);
        tx["timestamp"] = std::to_string(iter.timestamp);
        transactions.push_back(tx);
    }
    this->stored_chain[block_name]["transactions"] = transactions;
    std::ofstream fout(this->file_name);
    fout << this->stored_chain;
}

int BlockChain::add_block(Block b, zmq::socket_t &client){
    const std::lock_guard<std::mutex> lock(blockmutex);
    zmq::context_t &context = server.get_context();
    zmq::socket_t requester(context, ZMQ_REQ);
    requester.connect(metro_address);
    send_message(requester, CURRENT_PROBLEM);
    auto res1 = recv_message<Blake3Hash>(requester);
    Blake3Hash problem = res1.data;
#ifdef DEBUG
    printf("[DEBUG] Received a problem from metronome %s\n", base58_encode_key(problem).c_str());
#endif
    // compare the prev_hash of received block to hash of last block
    if(cmp_b3hash(problem, b.header.prev_hash)){
        /* TODO in future maybe print these */
        printf("[WARN] Received a block who's prev hash is not the current problem\n");
        send_message(client, STATUS_BAD);
        return -1;
    }
    if(!verify_block(b, consensus_type)){
        printf("[WARN] verify_block returned false, rejecting block\n");
        send_message(client, STATUS_BAD);
        return -1;
    }
#ifdef DEBUG
    printf("[DEBUG] Block vetted\n");
#endif
    send_message(client, STATUS_GOOD);
    send_message(requester, b, SUBMIT_BLOCK);
    auto res2 = recv_message<NullMessage>(requester);
    if(res2.header.type == STATUS_BAD){
        printf("[ERROR] received status bad from metronome\n");
        return -1;
    }
    else if(res2.header.type == REPLACE_BLOCK){
        /* happens when empty block and non empty block have same ID */
#ifdef DEBUG
        printf("[DEBUG] Received replace block message from metronome\n");
#endif
        blocks.pop_back();
    }

    blocks.push_back(b);
    write_block(b);
    sync_bal(b);
#ifdef DEBUG
    printf("[DEBUG] Successfully added block. Header displayed below\n");
    display_block_header(b.header);
#endif
    return 0;
}

int BlockChain::add_block(Block b){
    const std::lock_guard<std::mutex> lock(blockmutex);
    zmq::context_t &context = server.get_context();
    zmq::socket_t requester(context, ZMQ_REQ);
    requester.connect(metro_address);
    send_message(requester, CURRENT_PROBLEM);
    auto res1 = recv_message<Blake3Hash>(requester);
    Blake3Hash problem = res1.data;
#ifdef DEBUG
    printf("[DEBUG] Received a problem from metronome %s\n", base58_encode_key(problem).c_str());
#endif
    // compare the prev_hash of received block to hash of last block
    if(cmp_b3hash(problem, b.header.prev_hash)){
        /* TODO in future maybe print these */
        printf("[WARN] Received a block who's prev hash is not the current problem\n");
        return -1;
    }
    if(!verify_block(b, consensus_type)){
        printf("[WARN] verify_block returned false, rejecting block\n");
        return -1;
    }
#ifdef DEBUG
    printf("[DEBUG] Block vetted\n");
#endif
    send_message(requester, b, SUBMIT_BLOCK);
    auto res2 = recv_message<NullMessage>(requester);
    if(res2.header.type == STATUS_BAD){
        printf("[ERROR] received status bad from metronome\n");
        return -1;
    }
    else if(res2.header.type == REPLACE_BLOCK){
        /* happens when empty block and non empty block have same ID */
#ifdef DEBUG
        printf("[DEBUG] Received replace block message from metronome\n");
#endif
        blocks.pop_back();
    }

    blocks.push_back(b);
    write_block(b);
    sync_bal(b);
#ifdef DEBUG
    printf("[DEBUG] Successfully added block. Header displayed below\n");
    display_block_header(b.header);
#endif
    return 0;
}

int BlockChain::submit_block_peer(Block b, std::string peer_addr) {
    zmq::context_t& context = server.get_context();
    zmq::socket_t requester(context, ZMQ_REQ);

    /* connect timeout to 2000ms */
    requester.set(zmq::sockopt::connect_timeout, 2000);
    requester.connect(peer_addr); /* FIXME figure out what to do when timeout */

    send_message(requester, b, SUBMIT_BLOCK);
    auto response = recv_message<NullMessage>(requester);
    return response.header.type == STATUS_GOOD;
}

void BlockChain::submit_block(zmq::socket_t &client, MessageBuffer data) {
#ifdef DEBUG
    printf("[DEBUG] Received block\n");
#endif

    auto block = deserialize_payload<Block>(data);
    if(add_block(block, client) < 0){
        printf("[WARN] add_block failed in function submit_block; block header: %s\n", base58_encode_key(block.header.hash).c_str());
        return;
    }

#ifdef BLOCKCHAIN
    printf("Block added. [timestamp=%lu] [id=%d] [num_txs=%lu]\n", get_timestamp() / 1000000, block.header.id, block.transactions.size());
#endif
    zmq::context_t& context = server.get_context();
    zmq::socket_t requester(context, ZMQ_REQ);

    requester.connect(txpool_address);

    send_message(requester, block, CONFIRM_BLOCK);
    auto response = recv_message<NullMessage>(requester);

    for(std::string peer_addr : this->peers){
        /* TODO need to error handle
         * Make a map of peers -> queues
         * each thread will send blocks to the peer they are mapped to
         * we will put blocks on a queue in this loop
         * TODO make functions for adding new peers
         * TODO make function for deleting a peer
         */
        this->peer_threads->queue_job( [this, peer_addr, block] {
            printf("Sending block to peer %s\n", peer_addr.c_str());
            submit_block_peer(block, peer_addr);
        });
    }
}

void BlockChain::get_balance(zmq::socket_t &client, MessageBuffer data) {
    auto pub_key = deserialize_payload<Ed25519Key>(data);
    auto entry = this->ledger.find(pub_key);
    uint32_t balance = (entry == this->ledger.end()) ? 0 : entry->second;
    send_message(client, balance, STATUS_GOOD);
}

void BlockChain::last_block(zmq::socket_t &client, MessageBuffer data){
    Block b = this->blocks.back();
#ifdef DEBUG
    display_block_header(b.header);
#endif
    send_message(client, b, STATUS_GOOD);
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

void BlockChain::load_genesis() {
    Block genesis = get_genesis_block(this->config_file);
    this->blocks.push_back(genesis);
    sync_bal(genesis); // I'm pretty sure we need to sync bal after genesis
#ifdef DEBUG
    printf("Genesis block loaded\n");
#endif
}

void BlockChain::sync_chain() {
    zmq::context_t& context = server.get_context();
    zmq::socket_t requester(context, ZMQ_REQ);

    std::vector<std::string> temp;
    std::sample(peers.begin(), peers.end(), std::back_inserter(temp), 1, std::mt19937 {std::random_device{}()});
    std::string peer = temp[0];
    requester.connect(peer);

#ifdef DEBUG
    printf("[DEBUG] syncing with peer %s\n", peer.c_str());
#endif

    send_message(requester, QUERY_BLOCKS);
    auto response = recv_message<std::vector<Block>>(requester);
    std::vector<Block> recv_blocks = response.data;
    for(int i = 0; i < recv_blocks.size(); i++){
        Block block = recv_blocks[i];
        if(block.header.id == 0)
            continue; // * everyone should already have genesis block(block with id 0) * //
#ifdef DEBUG
        printf("[DEBUG] Attempting to submit block\n");
        display_block_header(block.header);
#endif
        if(add_block(block) < 0) {
            printf("[WARN] add_block failed in function sync_chain; block header: %s\n", base58_encode_key(block.header.hash).c_str());
            if(i > 1)
                printf("[WARN] Partial sync\n");
            else
                printf("[ERROR] sync failed\n");
            return;
        }
#ifdef DEBUG
        printf("[DEBUG] Successfully submited block\n");
#endif
    }
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

void BlockChain::query_blocks(zmq::socket_t &client, MessageBuffer data){
#ifdef DEBUG
    printf("[DEBUG] Received sync chain message, %lu blocks to send\n", this->blocks.size());
#endif
    send_message(client, this->blocks, STATUS_GOOD);
    return;
}

void BlockChain::get_metro_addr(zmq::socket_t &client, MessageBuffer data) {
    send_message(client, metro_address, STATUS_GOOD);
    return;
}

void BlockChain::get_peer_addrs(zmq::socket_t &client, MessageBuffer data) {
    send_message(client, peers, STATUS_GOOD);
    return;
}

void BlockChain::request_handler(zmq::socket_t &client, Message<MessageBuffer> request) {
    switch (request.header.type) {
        case QUERY_BAL:
#ifdef DEBUG
            printf("[DEBUG] QUERY_BAL\n");
#endif
            return get_balance(client, request.data);
        case SUBMIT_BLOCK:
#ifdef DEBUG
            printf("[DEBUG] SUBIMT_BLOCK\n");
#endif
            return submit_block(client, request.data);
        case QUERY_LAST_BLOCK:
#ifdef DEBUG
            printf("[DEBUG] QUERY_LAST_BLOCK\n");
#endif
            return last_block(client, request.data);
        case QUERY_TX_STATUS:
#ifdef DEBUG
            printf("[DEBUG] QUERY_TX_STATUS\n");
#endif
            return tx_status(client, request.data);
        case QUERY_NUM_ADDRS:
#ifdef DEBUG
            printf("[DEBUG] QUERY_NUM_ADDRS\n");
#endif
            return get_num_addr(client, request.data);
        case QUERY_COINS:
#ifdef DEBUG
            printf("[DEBUG] QUERY_COINS\n");
#endif
            return get_total_coins(client, request.data);
        case QUERY_BLOCKS:
#ifdef DEBUG
            printf("[DEBUG] QUERY_BLOCKS\n");
#endif
            return query_blocks(client, request.data);
        case GET_METRO_ADDR:
#ifdef DEBUG
            printf("[DEBUG] GET_METRO_ADDR\n");
#endif
            return get_metro_addr(client, request.data);
        case GET_PEER_ADDRS:
#ifdef DEBUG
            printf("[DEBUG] GET_PEER_ADDRS\n");
#endif
            return get_peer_addrs(client, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
