#include <cstdint>

#include "defs.hpp"
#include "wallet.hpp"

#pragma once

struct Transaction {

    enum Status {
        CONFIRMED,
        UNCONFIRMED,
        SUBMITTED,
        UNKNOWN
    };

    Ed25519Key src;
    Ed25519Key dest;
    Ed25519Signature signature;
    uint64_t id;
    uint32_t amount;
    uint64_t timestamp;
};

bool operator==(const Transaction& lhs, const Transaction& rhs);

Transaction create_transaction(Wallet src, Ed25519Key dest, uint32_t amount, uint64_t id); 
Transaction create_reward_transaction(Wallet miner);
void sign_transaction(Ed25519Key priv_key, Transaction &tx);
bool verify_transaction_signature(Transaction tx);
void display_transaction(Transaction tx);
int submit_transaction(Transaction tx, std::string tx_pool);
Transaction::Status query_transaction(std::string config_file, std::string key_file, uint64_t id, std::string tx_pool);
