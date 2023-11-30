#include <array>
#include <chrono>
#include <condition_variable>
#include <mutex>

#include "block.hpp"
#include "defs.hpp"
#include "metronome.hpp"
#include "messages.hpp"
#include "transaction.hpp"
#include "utils.hpp"

Metronome::Metronome(std::string _blockchain) {
    blockchain = _blockchain;
    difficulty = MIN_DIFFICULTY;
    last_block = request_last_block();
}

void Metronome::start(std::string address) {
	auto fp = std::bind(&Metronome::request_handler, this, std::placeholders::_1, std::placeholders::_2);
	
	if(server.start(address, fp, false) < 0)
		throw std::runtime_error("Server could not bind.");

    std::cv_status status;

    while(true) {
        printf("Waiting for block %d.\n", last_block.id + 1);

        // TODO: handle spurious wakeups
        std::unique_lock<std::mutex> lock(block_mutex);
        status = block_timer.wait_for(lock, std::chrono::seconds(BLOCK_TIME));

        update_difficulty(status == std::cv_status::timeout);

        if(status == std::cv_status::timeout) {
            printf("Submitting empty block.\n");
            submit_empty_block();
        }
    }
}

void Metronome::submit_empty_block() {

    Block empty_block = {
        .header = {
            .hash = {},
            .prev_hash = last_block.hash,
            .difficulty = difficulty,
            .timestamp = get_timestamp(),
            .id = last_block.id + 1,
        },
        .transactions = std::vector<Transaction>()
    };

    // randomly init hash
    srand(time(NULL));
    for (int i = 0; i < empty_block.header.hash.size(); i++)
        empty_block.header.hash[i] = rand() % 255;

    if(submit_block(empty_block) < 0) {
        printf("Empty block rejected from blockchain.\n");
        return;
    }

    // update last solved block state
    prev_solved_time = curr_solved_time;
    curr_solved_time = empty_block.header.timestamp;
    last_block = empty_block.header;
}

void Metronome::update_difficulty(bool timed_out) {
    int solved_time = curr_solved_time - prev_solved_time;

    if(timed_out) {
        if(difficulty <= MIN_DIFFICULTY) {
            printf("Block not solved in time. Difficulty already at minimum of %d.\n", difficulty);
            return;
        }

        std::unique_lock<std::mutex> diff_lock(diff_mutex);
        difficulty--;
        printf("Block not solved in time. Difficulty decreased to %d.\n", difficulty);
    } else if(solved_time < BLOCK_TIME / 2 * 1000000) {
        std::unique_lock<std::mutex> diff_lock(diff_mutex);
        difficulty++;
        printf("Block solved in %.3fs. Difficulty increased to %d.\n", float(solved_time) / 1000000, difficulty);
    } else {
        printf("Block solved in %.3fs. Difficulty remains at %d.\n", float(solved_time) / 1000000, difficulty);
    }
}

int Metronome::submit_block(Block block) {
    void* requester = zmq_socket(server.get_context(), ZMQ_REQ);
    zmq_connect(requester, blockchain.c_str());

    Message<NullMessage> response;
    request_response(requester, block, SUBMIT_BLOCK, response);

    zmq_close(requester);
    return response.type == STATUS_GOOD ? 0 : -1;
}

BlockHeader Metronome::request_last_block() {
    const int attempts = 12;
    const int timeout = 500;
    
    void* requester = zmq_socket(server.get_context(), ZMQ_REQ);
    zmq_connect(requester, blockchain.c_str());

    Message<BlockHeader> response;
    request_response(requester, QUERY_LAST_BLOCK, response);

    zmq_close(requester);
    return response.data;
}

void Metronome::handle_block(void* receiver, MessageBuffer data) {
    auto block = deserialize_payload<Block>(data);

    // TODO: validate block
    if(block.header.id != last_block.id + 1) {
        printf("Block not valid. Rejecting...\n");

        auto bytes = serialize_message(STATUS_BAD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
        return;
    }

    if(submit_block(block) < 0) {
        printf("Block rejected from blockchain.\n");

        auto bytes = serialize_message(STATUS_BAD);
        zmq_send (receiver, bytes.data(), bytes.size(), 0);
        return;
    }

    // Notify block timer a solution has been accepted
    std::unique_lock<std::mutex> lock(block_mutex);
    prev_solved_time = curr_solved_time;
    curr_solved_time = block.header.timestamp;
    last_block = block.header;
    block_timer.notify_one();

    auto bytes = serialize_message(STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void Metronome::get_difficulty(void* receiver, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(diff_mutex);
    auto bytes = serialize_message(difficulty, STATUS_GOOD);
    zmq_send (receiver, bytes.data(), bytes.size(), 0);
}

void Metronome::request_handler(void* receiver, Message<MessageBuffer> request) {
    switch (request.type) {
        case SUBMIT_BLOCK:
            return handle_block(receiver, request.data);
        case QUERY_DIFFICULTY:
            return get_difficulty(receiver, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}