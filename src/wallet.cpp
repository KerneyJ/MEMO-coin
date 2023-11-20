#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <array>
#include <iomanip> 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "keys.hpp"

Wallet create_wallet() {
    Ed25519Key pub_key, priv_key;
    gen_keys_ed25519(pub_key, priv_key);
    
    return { priv_key, pub_key };
}

// Function to load a wallet from a file
bool load_wallet(Wallet &wallet) {
    std::string pub_key, priv_key;
    std::ifstream file(WALLET_PATH);

    if (file.is_open()) {
        if(!std::getline(file, pub_key))
            return false;

        if(!std::getline(file, priv_key))
            return false;

        wallet.pub_key = base58_decode(pub_key);
        wallet.priv_key = base58_decode(priv_key);

        file.close();
    } else {
        printf("Error opening file: %s\n", WALLET_PATH);
    }

    return true;
}


// Function to write a wallet to a file
bool store_wallet(Wallet& wallet) {
    std::ofstream file(WALLET_PATH);

    if (file.is_open()) {
        std::string pub_key = base58_encode(wallet.pub_key);
        std::string priv_key = base58_encode(wallet.priv_key);
        
        file.write(pub_key.data(), pub_key.size());
        file.write("\n", 1);
        file.write(priv_key.data(), priv_key.size());
        file.close();
    } else {
        printf("Error opening file: %s\n", WALLET_PATH);
        return false;
    }

    return true;
}

void display_wallet(Wallet& wallet) {
    std::string pub_key = base58_encode(wallet.pub_key);
    std::string priv_key = base58_encode(wallet.priv_key);

    printf("Public Key: %s\n", pub_key.c_str());
    printf("Private Key: %s\n", priv_key.c_str());
}