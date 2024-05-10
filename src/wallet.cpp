#include <array>
#include <cstdint>
#include <fstream>
#include <zmq.hpp>

#include "config.hpp"
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

int query_balance(std::string blockchain_node, std::string config_file, std::string key_file) {
    Wallet wallet;

    load_wallet(wallet, config_file, key_file);

    zmq::context_t context;
    zmq::socket_t requester(context, ZMQ_REQ);

    requester.connect(blockchain_node);

    send_message(requester, wallet.pub_key, QUERY_BAL);
    auto response = recv_message<uint32_t>(requester);

    return response.data;
}
