#include "messages.hpp"
#include <cstddef>

Message<MessageBuffer> recv_message_z(zmq::socket_t &client) {
    Message<MessageBuffer> message;
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
    message.data = payload_buf;

    return message;
}