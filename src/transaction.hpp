#include <cstdint>

#include "defs.hpp"
#include "wallet.hpp"

#pragma once

struct Transaction {
    Ed25519Key src;
    Ed25519Key dest;
    Ed25519Signature signature;
    uint64_t id;
    uint32_t amount;
    uint64_t timestamp;
};

Transaction create_transaction(Wallet src, Ed25519Key dest, uint32_t amount, uint64_t id); 
void sign_transaction(Ed25519Key priv_key, Transaction &tx);
bool verify_transaction_signature(Ed25519Key pub_key, Transaction tx);
void display_transaction(Transaction tx);