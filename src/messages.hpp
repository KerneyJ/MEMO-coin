#include <array>
#include <cstdint>
#include <vector>
#include <alpaca/alpaca.h>

#include "transaction.hpp"

#define MESSAGE_BUF_SIZE 1000
#define MESSAGE_SIZE sizeof(Message<MessageBuffer>)


enum message_type {
    POST_TX,
    POP_TX,
    STATUS_GOOD,
    STATUS_BAD,
};

template<typename Type>
struct Message {
    message_type type;
    Type buffer;
};

typedef std::array<uint8_t, MESSAGE_BUF_SIZE> MessageBuffer;
typedef std::array<uint8_t, MESSAGE_SIZE> ReceiveBuffer;

constexpr auto OPTIONS = alpaca::options::fixed_length_encoding;

template<typename PayloadType>
std::vector<uint8_t> serialize_message(PayloadType data, message_type type) {
    std::vector<uint8_t> bytes;
    Message<PayloadType> msg = { type, data };
    auto bytes_written = alpaca::serialize<OPTIONS>(msg, bytes);
    return bytes; 
}

template<typename PayloadType>
Message<PayloadType> deserialize_message(std::array<uint8_t, MESSAGE_SIZE> bytes) {
    std::error_code ec;
    return alpaca::deserialize<OPTIONS, Message<PayloadType>>(bytes, ec); 
}

template<typename PayloadType>
PayloadType deserialize_payload(std::array<uint8_t, MESSAGE_BUF_SIZE> bytes) {
    std::error_code ec;
    return alpaca::deserialize<OPTIONS, PayloadType>(bytes, ec); 
}

class NullMessage {};