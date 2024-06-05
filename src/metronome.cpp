#include <array>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <zmq.hpp>

#include "block.hpp"
#include "defs.hpp"
#include "metronome.hpp"
#include "messages.hpp"
#include "transaction.hpp"
#include "utils.hpp"

Metronome::Metronome(std::string _blockchain, std::string _consensus_type) {
    blockchains.push_back(_blockchain);
    consensus_type = _consensus_type;
    difficulty = MIN_DIFFICULTY;
    sleeping = true;
    last_block = request_last_block();
    display_block_header(last_block.header);
    active_validators = 0;
}

void Metronome::start(std::string address) {
    // TODO have the metronome deterime who gets to pull transaction from the transaction pool
    auto fp = std::bind(&Metronome::request_handler, this, std::placeholders::_1, std::placeholders::_2);

    if(server.start(address, fp, false) < 0)
        throw std::runtime_error("Server could not bind.");

    std::cv_status status;
    std::chrono::system_clock::time_point block_deadline;

    while(true) {
#ifdef DEBUG
        printf("[DEBUG] Waiting for block %d.\n", last_block.header.id + 1);
#endif
        {
            std::unique_lock<std::mutex> lock(block_mutex);
            block_deadline = std::chrono::system_clock::time_point(
                std::chrono::microseconds(last_block.header.timestamp) + std::chrono::seconds(BLOCK_TIME));
            status = std::cv_status::no_timeout;

            while(sleeping && status == std::cv_status::no_timeout)
                status = block_timer.wait_until(lock, block_deadline);

            sleeping = true;
        }

        update_difficulty(status == std::cv_status::timeout);

        if(status == std::cv_status::timeout) {
#ifdef DEBUG
            printf("[DEBUG] Submitting empty block.\n");
#endif
            submit_empty_block();
        }
    }
}

void Metronome::submit_empty_block() {
    /* std::unique_lock<std::mutex> lock(block_mutex); */

    Block empty_block = {
        .header = {
            .hash = {},
            .prev_hash = last_block.header.hash,
            .difficulty = difficulty,
            .timestamp = get_timestamp(),
            .id = last_block.header.id + 1,
        },
        .transactions = std::vector<Transaction>()
    };

    // randomly init hash
    srand(time(NULL));
    for (int i = 0; i < empty_block.header.hash.size(); i++)
        empty_block.header.hash[i] = rand() % 255;

    block_mutex.lock();
    if(submit_block(empty_block) < 0) {
#ifdef DEBUG
        printf("Empty block rejected from blockchain.\n");
#endif
        return;
    }

    // update last solved block state
    prev_solved_time = curr_solved_time;
    curr_solved_time = empty_block.header.timestamp;
    last_block = empty_block;
    block_mutex.unlock();
}
/*
void sync_chain() {
    zmq::context_t& context = server.get_context();
    zmq::socket_t blockchain_req(context, ZMQ_REQ);
    zmq::socket_t peer_req(context, ZMQ_REQ);

    // * get vector of peers from blockchain node
     * FIXME if there are multiple blockchain nodes
     * then we need to pick one from the vector
     * //
    blockchain_req.connect(blockchain);
    send_message(blockchain_req, GET_PEER_ADDRS);
    auto res1 = recv_message<std::vector<std::string>>(blockchain_req);
    std::vector<std::string> peers = res1.data;
    std::string peer = peers[0]; // * TODO probably get random peer in future * //

    // * get vector of blocks from blockchain node * //
    peer_req.connect(peer);
    send_message(peer_req, QUERY_BLOCKS);
    auto res2 = recv_message<std::vector<Block>>(peer_req);
    if(res2.header.type != STATUS_GOOD) {
        printf("[ERROR] received non good status when synching with peer %s\n", peer.c_str());
        exit(1);
    }
    std::vector<Block> blocks = res2.data;
    for(int i = 0; i < blocks.size(); i++) {
        Block block = blocks[i];
        if(block.header.id == 0)
            continue; // * everyone should already have genesis block(block with id 0) * //
#ifdef DEBUG
        printf("[DEBUG] Attempting to submit block\n");
        display_block_header(block.header);
#endif
        // TODO need to validate the vector of blocks we received
        if(submit_block(block) < 0) {
            printf("[ERROR] Block rejected from blockchain.\n");
            exit(1);
        }
#ifdef DEBUG
        printf("[DEBUG] successfully submited block\n");
#endif
    }
}
*/
void Metronome::update_difficulty(bool timed_out) {
    int solved_time = curr_solved_time - prev_solved_time;

    if(timed_out) {
        if(difficulty <= MIN_DIFFICULTY) {
#ifdef DEBUG
            printf("Block not solved in time. Difficulty already at minimum of %d.\n", difficulty);
#endif
            return;
        }

        std::unique_lock<std::mutex> diff_lock(diff_mutex);
        difficulty--;
#ifdef DEBUG
        printf("Block not solved in time. Difficulty decreased to %d.\n", difficulty);
#endif
    } else if(solved_time < BLOCK_TIME / 2 * 1000000) {
        std::unique_lock<std::mutex> diff_lock(diff_mutex);
        difficulty++;
#ifdef DEBUG
        printf("Block solved in %.3fs. Difficulty increased to %d.\n", float(solved_time) / 1000000, difficulty);
#endif
    } else {
#ifdef DEBUG
        printf("Block solved in %.3fs. Difficulty remains at %d.\n", float(solved_time) / 1000000, difficulty);
#endif
    }
}

int Metronome::submit_block(Block block) {
    for(std::string blockchain : blockchains) {
        zmq::socket_t requester(server.get_context(), ZMQ_REQ);
        requester.connect(blockchain);
        send_message(requester, block, SUBMIT_BLOCK);
        auto response = recv_message<NullMessage>(requester);
    }
    return 0; /* TODO add debug stuff */
    /* return response.header.type == STATUS_GOOD ? 0 : -1; */
}

Block Metronome::request_last_block() {
    zmq::socket_t requester(server.get_context(), ZMQ_REQ);
    requester.connect(blockchains[0]);

    send_message(requester, QUERY_LAST_BLOCK);
    auto response = recv_message<Block>(requester);

    return response.data;
}

void Metronome::handle_block(zmq::socket_t &client, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(block_mutex);
    auto block = deserialize_payload<Block>(data);

    /* TODO check other conditions
     * - if block is empty immediate reject
     * - if last block is empty and block is not empty then replace block with last block
     * - if the last block id == block id then find some way to compare and deterministically choose a block (maybe create a signature and cmopare and get lowest signature)
     */
    if(!verify_block(block, consensus_type)){
        printf("[WARN] verify_block returned false, rejecting block\n");
        send_message(client, STATUS_BAD);
    }
    else if(block.transactions.empty()){
#ifdef DEBUG
        printf("[DEBUG] received empty block\n");
#endif
        send_message(client, STATUS_GOOD);
    }
    else if(block.header.id == last_block.header.id && last_block.transactions.empty()){
        /* replace last_block with block */
#ifdef DEBUG
        printf("[DEBUG] Received a block that has same id as last block\n");
#endif
        if(last_block.transactions.empty()){
#ifdef DEBUG
            printf("[DEBUG] Last block is empty and received non-empty block with same ID, sending replace block message\n");
#endif
            send_message(client, REPLACE_BLOCK);
        }
        else{
            /* TODO find a way to deterministically choose the same block right now sending status bad
             * TODO Also, since, the last block is not empty we need a way to role back all of those transaction */
#ifdef DEBUG
            printf("[DEBUG] TODO got blocks of equal id\n");
#endif
            send_message(client, STATUS_BAD);
        }
    }
    else if(block.header.id == last_block.header.id + 1){
        /* just submit the block */
        send_message(client, STATUS_GOOD);
    }
    else{
        printf("[ERROR] block id not equal to last block id or last bloc id + 1\n");
        printf("\t\t last block id: %lu; received block id: %lu\n", last_block.header.id, block.header.id);
        send_message(client, STATUS_BAD);
    }

    /* Notify block timer a solution has been accepted */
    prev_solved_time = curr_solved_time;
    curr_solved_time = block.header.timestamp;
    last_block = block;
    sleeping = false;
    block_timer.notify_one();
    /* send_message(client, STATUS_GOOD); */
}

void Metronome::get_difficulty(zmq::socket_t &client, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(diff_mutex);
    send_message(client, difficulty, STATUS_GOOD);
}

//increments the number of active_validators every time a new validator attempts to mine the block. 
//active_validators is reset at each block so the accumulates over the duration of a block (e.g. 6 seconds).
//TODO: keep a list of active validator addresses. Validator should send its wallet public key.
void Metronome::register_validator(zmq::socket_t &client, MessageBuffer data) {
    this->active_validators += 1;
#ifdef DEBUG
    printf("Registered new validator [%d]\n", this->active_validators);
#endif
    send_message(client, STATUS_GOOD);
}

void Metronome::query_validators(zmq::socket_t &client, MessageBuffer data) {
#ifdef DEBUG
    printf("Registered new validator [%d]\n", this->active_validators);
#endif
    send_message(client, active_validators, STATUS_GOOD);
}

void Metronome::current_problem(zmq::socket_t &client, MessageBuffer data) {
    send_message(client, last_block.header.hash, STATUS_GOOD);
}

void Metronome::register_blockchain(zmq::socket_t &client, MessageBuffer data) {
    auto blockchain_addr = deserialize_payload<std::string>(data);
    blockchains.push_back(blockchain_addr);
    send_message(client, STATUS_GOOD);
}

void Metronome::request_handler(zmq::socket_t &client, Message<MessageBuffer> request) {
    switch (request.header.type) {
        case SUBMIT_BLOCK:
            return handle_block(client, request.data);
        case QUERY_DIFFICULTY:
            return get_difficulty(client, request.data);
        case REGISTER_VALIDATOR:
            return register_validator(client, request.data);
        case QUERY_NUM_VALIDATORS:
            return query_validators(client, request.data);
        case CURRENT_PROBLEM: // TODO maybe delete this function(different way of submitting blocks)
            return current_problem(client, request.data);
        case REGISTER_BLOCKCHAIN:
            return register_blockchain(client, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
