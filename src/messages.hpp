#include <array>
#include <cstdint>
#include <vector>
#include <alpaca/alpaca.h>

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
};

template<typename Type>
struct Message {
    MessageType type;
    Type buffer;
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
