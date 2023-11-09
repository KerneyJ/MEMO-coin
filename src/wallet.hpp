#include <cstdint>
#include <string>
#include <array>

#include "defs.hpp"
#include "transaction.hpp"

#pragma once

struct Wallet {
    SHA256Hash priv_key;
    SHA256Hash pub_key;
};

Wallet create_wallet(std::string filepath);
Wallet load_wallet(std::string filepath);
void store_wallet(std::string filepath, Wallet wallet);
Transaction create_transaction(SHA256Hash src, SHA256Hash dest, uint32_t amount); 

// TODO: later when we have other nodes
uint32_t query_balance();
void submit_transaction();