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

Metronome::Metronome(std::string _blockchain) {
    blockchain = _blockchain;
    difficulty = MIN_DIFFICULTY;
    sleeping = true;
    last_block = request_last_block();
    display_block_header(last_block);
    active_validators = 0;
}

void Metronome::start(std::string address) {
	auto fp = std::bind(&Metronome::request_handler, this, std::placeholders::_1, std::placeholders::_2);
	
	if(server.start(address, fp, false) < 0)
		throw std::runtime_error("Server could not bind.");

    std::cv_status status;
    std::chrono::system_clock::time_point block_deadline;

    while(true) {
#ifdef DEBUG
        printf("Waiting for block %d.\n", last_block.id + 1);
#endif
        {
            std::unique_lock<std::mutex> lock(block_mutex);
            block_deadline = std::chrono::system_clock::time_point(
                std::chrono::microseconds(last_block.timestamp) + std::chrono::seconds(BLOCK_TIME));
            status = std::cv_status::no_timeout;

            while(sleeping && status == std::cv_status::no_timeout)
                status = block_timer.wait_until(lock, block_deadline);
            
            sleeping = true;
        }

        update_difficulty(status == std::cv_status::timeout);

        if(status == std::cv_status::timeout) {
#ifdef DEBUG
            printf("Submitting empty block.\n");
#endif
            submit_empty_block();
        }
    }
}

void Metronome::submit_empty_block() {
    std::unique_lock<std::mutex> lock(block_mutex);

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
#ifdef DEBUG
        printf("Empty block rejected from blockchain.\n");
#endif
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
    zmq::socket_t requester(server.get_context(), ZMQ_REQ);
    requester.connect(blockchain);

    send_message(requester, block, SUBMIT_BLOCK);
    auto response = recv_message<NullMessage>(requester);

    return response.header.type == STATUS_GOOD ? 0 : -1;
}

BlockHeader Metronome::request_last_block() {
    zmq::socket_t requester(server.get_context(), ZMQ_REQ);
    requester.connect(blockchain);

    send_message(requester, QUERY_LAST_BLOCK);
    auto response = recv_message<BlockHeader>(requester);

    return response.data;
}

void Metronome::handle_block(zmq::socket_t &client, MessageBuffer data) {
    std::unique_lock<std::mutex> lock(block_mutex);
    auto block = deserialize_payload<Block>(data);

    // TODO: validate block
    if(block.header.id != last_block.id + 1) {
        printf("Block not valid. Rejecting...\n");
        send_message(client, STATUS_BAD);
        return;
    }

    if(submit_block(block) < 0) {
        printf("Block rejected from blockchain.\n");
        send_message(client, STATUS_BAD);
        return;
    }

    // Notify block timer a solution has been accepted
    prev_solved_time = curr_solved_time;
    curr_solved_time = block.header.timestamp;
    last_block = block.header;
    sleeping = false;
    block_timer.notify_one();

    send_message(client, STATUS_GOOD);
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

void Metronome::request_handler(zmq::socket_t &client, Message<MessageBuffer> request) {
    switch (request.header.type) {
        case SUBMIT_BLOCK:
            return handle_block(client, request.data);
        case QUERY_DIFFICULTY:
            return get_difficulty(client, request.data);
        case REGISTER_VALIDATOR:
            return register_validator(client, request.data);
        // case QUERY_NUM_VALIDATORS:
        //     return register_validator(client, request.data);
        default:
            throw std::runtime_error("Unknown message type.");
    }
}
