#include <array>
#include <cstdint>

#include "transaction.hpp"

#define MESSAGE_SIZE 1000

enum message_type {
    POST_TX,
    POP_TX
};

struct Message {
    message_type type;
    std::array<uint8_t, MESSAGE_SIZE> buffer;
};