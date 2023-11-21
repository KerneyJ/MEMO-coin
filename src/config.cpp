#include <fstream>
#include <yaml-cpp/yaml.h>

#include "wallet.hpp"
#include "config.hpp"
#include "keys.hpp"

bool load_wallet(Wallet &wallet) {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    YAML::Node private_config = YAML::LoadFile(PRIVATE_KEY_FILE);

    const std::string pub_key = config["wallet"]["public_key"].as<std::string>();
    const std::string priv_key = private_config["wallet"]["private_key"].as<std::string>();

    wallet.pub_key = base58_decode(pub_key);
    wallet.priv_key = base58_decode(priv_key);

    return true;
}

bool store_wallet(Wallet &wallet) {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    YAML::Node private_config = YAML::LoadFile(PRIVATE_KEY_FILE);

    config["wallet"]["public_key"] = base58_encode(wallet.pub_key);
    private_config["wallet"]["private_key"] =  base58_encode(wallet.priv_key);

    std::ofstream conf_out(CONFIG_FILE);
    std::ofstream priv_conf_out(PRIVATE_KEY_FILE);
    
    conf_out << config;
    priv_conf_out << private_config;

    return true;
}

uint64_t get_tx_id() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);

    const uint64_t nonce = config["wallet"]["nonce"].as<uint64_t>();
    config["wallet"]["nonce"] = nonce+1;

    std::ofstream fout(CONFIG_FILE);
    fout << config;

    return nonce;
}