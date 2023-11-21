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
    std::string pub_key = base58_encode(wallet.pub_key);
    std::string priv_key = base58_encode(wallet.priv_key);

    printf("Public Key: %s\n", pub_key.c_str());
    printf("Private Key: %s\n", priv_key.c_str());
}

int query_balance(std::string blockchain_node) {
    return -1;
}