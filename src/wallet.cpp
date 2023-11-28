#include <array>
#include <cstdint>
#include <fstream>
#include <zmq.h>

#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "keys.hpp"
#include "messages.hpp"

Wallet create_wallet() {
    Ed25519Key pub_key, priv_key;
    gen_keys_ed25519(pub_key, priv_key);
    
    return { priv_key, pub_key };
}

void display_wallet(Wallet& wallet) {
    std::string pub_key = base58_encode_key(wallet.pub_key);
    std::string priv_key = base58_encode_key(wallet.priv_key);

    printf("Public Key: %s\n", pub_key.c_str());
    printf("Private Key: %s\n", priv_key.c_str());
}

int query_balance(std::string blockchain_node) {
    Wallet wallet;

    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);

    zmq_connect(requester, blockchain_node.c_str());

    Message<uint32_t> response;
    request_response(requester, wallet.pub_key, QUERY_BAL, response);

    zmq_close(requester);
    zmq_ctx_destroy(context);

    return response.data;
}
