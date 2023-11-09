#include <vector>
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <alpaca/alpaca.h>

#include "data.hpp"

int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    std::array<uint8_t, 40> bytes;
    zmq_recv (responder, bytes.data(), 40, 0);
        
    std::error_code ec;
    Bar deserialized_bar = alpaca::deserialize<Bar>(bytes, ec);
    printf("deserialized data:\n");
    printf("type: %d\n", deserialized_bar.type);
    printf("foo1: (%d, %c, %s)\n", deserialized_bar.foos[0].a, deserialized_bar.foos[0].b, deserialized_bar.foos[0].c.c_str());
    printf("foo2: (%d, %c, %s)\n", deserialized_bar.foos[1].a, deserialized_bar.foos[1].b, deserialized_bar.foos[1].c.c_str());
    printf("foo3: (%d, %c, %s)\n", deserialized_bar.foos[2].a, deserialized_bar.foos[2].b, deserialized_bar.foos[2].c.c_str());
    
    return 0;
}