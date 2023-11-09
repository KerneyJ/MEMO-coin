#include <cstdint>
#include <string>
#include <array>

#include "defs.hpp"

#pragma once

struct Wallet {
    SHA256Hash priv_key;
    SHA256Hash pub_key;
    SHA256Hash id;
};

Wallet load_wallet(std::string filepath);
void store_wallet(std::string filepath, Wallet wallet);