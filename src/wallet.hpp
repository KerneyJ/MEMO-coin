#include <cstdint>
#include <string>
#include <array>

#include "defs.hpp"
#include "transaction.hpp"
#include "defs.hpp"

#pragma once

struct Wallet {
    Ed25519Key priv_key;
    Ed25519Key pub_key;
};

Wallet create_wallet();
Wallet load_wallet(std::string filepath);
void store_wallet(std::string filepath, Wallet wallet);
Transaction create_transaction(Ed25519Key src, Ed25519Key dest, uint32_t amount); 

// TODO: later when we have other nodes
uint32_t query_balance();
void submit_transaction();