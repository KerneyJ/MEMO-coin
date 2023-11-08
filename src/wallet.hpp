#include <cstdint>
#include <string>

struct Wallet {
    uint8_t priv_key[32];
    uint8_t pub_key[32];
    uint8_t id[32];
};

Wallet load_wallet(std::string filepath);
void store_wallet(std::string filepath, Wallet wallet);