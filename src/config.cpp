#include <fstream>
#include <yaml-cpp/yaml.h>

#include "wallet.hpp"
#include "config.hpp"
#include "keys.hpp"

// Wallet config

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

// Trancastion pool config

std::string get_tx_pool_address() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["tx_pool"]["address"].as<std::string>();
}

std::string get_tx_pool_threads() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["tx_pool"]["address"].as<std::string>();
}

// Validator config

std::string get_validator_address() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["validator"]["address"].as<std::string>();
}

std::string get_validator_threads() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["validator"]["address"].as<std::string>();
}

std::string get_consensus_method() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["validator"]["consensus"].as<std::string>();
}

// Metronome config

std::string get_metronome_address() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["metronome"]["address"].as<std::string>();
}

std::string get_metronome_threads() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["metronome"]["address"].as<std::string>();
}

// Blockchain config

std::string get_blockchain_address() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["blockchain"]["address"].as<std::string>();
}

std::string get_blockchain_threads() {
    YAML::Node config = YAML::LoadFile(CONFIG_FILE);
    return config["blockchain"]["address"].as<std::string>();
}
