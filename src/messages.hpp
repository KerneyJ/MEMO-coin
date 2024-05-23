#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <system_error>
#include <vector>
#include <alpaca/alpaca.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "transaction.hpp"

#pragma once

#define MESSAGE_BUF_SIZE 1000
#define MESSAGE_SIZE sizeof(Message<MessageBuffer>)

enum MessageType {
    POST_TX,
    POP_TX,
    QUERY_TX_STATUS,
    STATUS_GOOD,
    STATUS_BAD,
    QUERY_BAL,
    SUBMIT_BLOCK,
    QUERY_DIFFICULTY,
    QUERY_LAST_BLOCK,
    CONFIRM_BLOCK,
    QUERY_TX_COUNT,
    QUERY_NUM_ADDRS,
    QUERY_COINS,
    REGISTER_VALIDATOR,
    QUERY_NUM_VALIDATORS,
    SYNC_CHAIN,
};

struct MessageHeader {
    enum MessageType type;
    size_t size;
};

/* Don't use this. This is to ensure any type is serializable. */
template<typename Type>
struct MessagePayload {
    Type data;
};

template<typename Type>
struct Message {
    MessageHeader header;
    Type data;
};

class NullMessage {};

typedef std::vector<uint8_t> MessageBuffer;

constexpr auto OPTIONS = alpaca::options::fixed_length_encoding;

template<typename PayloadType>
void send_message(zmq::socket_t &client, PayloadType message, MessageType type) {
    std::vector<uint8_t> payload_bytes, header_bytes;
    MessagePayload<PayloadType> payload = { message };

    alpaca::serialize<OPTIONS>(payload, payload_bytes);

    MessageHeader header = {
        .type = type,
        .size = payload_bytes.size(),
    };

    alpaca::serialize<OPTIONS>(header, header_bytes);

    std::array<zmq::const_buffer, 2> multipart_msg { 
        zmq::buffer(header_bytes), 
        zmq::buffer(payload_bytes),
    };

    zmq::send_multipart(client, multipart_msg);
}

inline void send_message(zmq::socket_t &client, MessageType type) {
    send_message<NullMessage>(client, {}, type);
}

template<typename PayloadType>
Message<PayloadType> recv_message(zmq::socket_t &client) {
    Message<PayloadType> message;
    std::error_code ec;
    std::vector<zmq::message_t> recv_msgs;

    const auto ret = zmq::recv_multipart(client, std::back_inserter(recv_msgs));

    if(ret != 2)
        throw std::runtime_error("Received malformed message.");

    uint8_t* header_loc = (uint8_t*) recv_msgs[0].data();
    size_t header_size = recv_msgs[0].size();
    std::vector<uint8_t> header_buf(&header_loc[0], &header_loc[header_size]);


    uint8_t* payload_loc = (uint8_t*) recv_msgs[1].data();
    size_t payload_size = recv_msgs[1].size();
    std::vector<uint8_t> payload_buf(&payload_loc[0], &payload_loc[payload_size]);

    message.header = alpaca::deserialize<OPTIONS, MessageHeader>(header_buf, ec);
    message.data = alpaca::deserialize<OPTIONS, MessagePayload<PayloadType>>(payload_buf, ec).data;

    return message;
}

Message<MessageBuffer> recv_message(zmq::socket_t &client);

template<typename PayloadType>
PayloadType deserialize_payload(MessageBuffer bytes) {
    std::error_code ec;
    return alpaca::deserialize<OPTIONS, MessagePayload<PayloadType>>(bytes, ec).data; 
}
