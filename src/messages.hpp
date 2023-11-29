#include <array>
#include <cstdint>
#include <vector>
#include <alpaca/alpaca.h>
#include <zmq.h>

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
    QUERY_TX_COUNT
};

template<typename Type>
struct Message {
    MessageType type;
    Type data;
};

class NullMessage {};

typedef std::array<uint8_t, MESSAGE_BUF_SIZE> MessageBuffer;
typedef std::array<uint8_t, MESSAGE_SIZE> ReceiveBuffer;

constexpr auto OPTIONS = alpaca::options::fixed_length_encoding;

template<typename PayloadType>
std::vector<uint8_t> serialize_message(PayloadType data, MessageType type) {
    std::vector<uint8_t> bytes;
    Message<PayloadType> msg = { type, data };
    auto bytes_written = alpaca::serialize<OPTIONS>(msg, bytes);
    return bytes; 
}

// use for messages that do not need any response data
inline std::vector<uint8_t> serialize_message(MessageType type) {
    return serialize_message<NullMessage>({}, type);
}

template<typename PayloadType>
Message<PayloadType> deserialize_message(std::array<uint8_t, MESSAGE_SIZE> bytes){
    std::error_code ec;
    return alpaca::deserialize<OPTIONS, Message<PayloadType>>(bytes, ec); 
}

template<typename PayloadType>
PayloadType deserialize_payload(std::array<uint8_t, MESSAGE_BUF_SIZE> bytes) {
    std::error_code ec;
    return alpaca::deserialize<OPTIONS, PayloadType>(bytes, ec); 
}

template<typename RequestType, typename ResponseType>
void request_response(void* client, RequestType request, MessageType type, Message<ResponseType> &response) {
    ReceiveBuffer res_buf;

    auto req_buf = serialize_message(request, type);
    zmq_send(client, req_buf.data(), req_buf.size(), 0);
    zmq_recv(client, res_buf.data(), res_buf.size(), 0);
    response = deserialize_message<ResponseType>(res_buf);
}

template<typename ResponseType>
void request_response(void* client, MessageType type, Message<ResponseType> &response) {
    ReceiveBuffer res_buf;

    auto req_buf = serialize_message(type);
    zmq_send(client, req_buf.data(), req_buf.size(), 0);
    zmq_recv(client, res_buf.data(), res_buf.size(), 0);
    response = deserialize_message<ResponseType>(res_buf);
}