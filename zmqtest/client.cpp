#include <array>
#include <string>
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <alpaca/alpaca.h>
#include "data.hpp"

int main (void)
{
    printf ("Connecting to hello world server…\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    Foo foo1 = {42, 'J', "test1234"};
    Foo foo2 = {69, 'K', "woop woop"};
    Foo foo3 = {420, 'L', "jajajajaja"};
    Bar my_bar = {OKAY, {foo1, foo2, foo3}};

    std::vector<uint8_t> bytes;
    auto bytes_written = alpaca::serialize(my_bar, bytes);
    printf("bytes written: %lu\n", bytes_written);

    printf ("Sending Struct…\n");
    zmq_send (requester, bytes.data(), bytes.size(), 0);

    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}