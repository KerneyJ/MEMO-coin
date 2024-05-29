#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include "wallet.hpp"
#include "config.hpp"
#include "keys.hpp"

// global config
bool get_sync_chain(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);

    bool sync_chain = config["sync_chain"].as<bool>();

    return sync_chain;
}

// Wallet config

bool load_wallet(Wallet& wallet, std::string config_file, std::string key_file){
    YAML::Node config = YAML::LoadFile(config_file);
    YAML::Node private_config = YAML::LoadFile(key_file);

    const std::string pub_key = config["wallet"]["public_key"].as<std::string>();
    const std::string priv_key = private_config["wallet"]["private_key"].as<std::string>();

    wallet.pub_key = base58_decode_key(pub_key);
    wallet.priv_key = base58_decode_key(priv_key);

    return true;
}

bool store_wallet(Wallet& wallet, std::string config_file, std::string key_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    YAML::Node private_config = YAML::LoadFile(key_file);

    config["wallet"]["public_key"] = base58_encode_key(wallet.pub_key);
    private_config["wallet"]["private_key"] =  base58_encode_key(wallet.priv_key);

    std::ofstream conf_out(config_file);
    std::ofstream priv_conf_out(key_file);
    
    conf_out << config;
    priv_conf_out << private_config;

    return true;
}

uint64_t get_tx_id(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);

    const uint64_t nonce = config["wallet"]["nonce"].as<uint64_t>();
    config["wallet"]["nonce"] = nonce+1;

    std::ofstream fout(config_file);
    fout << config;

    return nonce;
}

void set_tx_id(uint64_t nonce, std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);

    config["wallet"]["nonce"] = nonce;

    std::ofstream fout(config_file);
    fout << config;
}

// Trancastion pool config

std::string get_tx_pool_address(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["tx_pool"]["address"].as<std::string>();
}

std::string get_tx_pool_threads(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["tx_pool"]["address"].as<std::string>();
}

// Validator config

std::string get_validator_address(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["validator"]["address"].as<std::string>();
}

std::string get_validator_threads(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["validator"]["address"].as<std::string>();
}

std::string get_consensus_method(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["validator"]["consensus"].as<std::string>();
}

uint32_t get_validator_memory(std::string config_file) {
    int base_val, num_end;
    std::string str_val;

    YAML::Node config = YAML::LoadFile(config_file);

    str_val = config["validator"]["memory"].as<std::string>();

    if((num_end = str_val.find("KB")) != std::string::npos) {
        return std::stoi(str_val.substr(0, num_end)) * 1000;
    }

    if((num_end = str_val.find("MB")) != std::string::npos) {
        return std::stoi(str_val.substr(0, num_end)) * 1000000;
    }

    if((num_end = str_val.find("GB")) != std::string::npos) {
        return std::stoi(str_val.substr(0, num_end)) * 1000000000;
    }

    throw std::runtime_error("Validator memory improperly formatted.");
}

void set_validator_fingerprint(UUID fingerprint, std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);

    config["validator"]["fingerprint"] = base58_encode_uuid(fingerprint);

    std::ofstream fout(config_file);
    fout << config;
}

// Metronome config

std::string get_metronome_address(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["metronome"]["address"].as<std::string>();
}

std::string get_metronome_threads(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["metronome"]["address"].as<std::string>();
}

// Blockchain config

std::string get_blockchain_address(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["blockchain"]["address"].as<std::string>();
}

std::string get_blockchain_threads(std::string config_file) {
    YAML::Node config = YAML::LoadFile(config_file);
    return config["blockchain"]["address"].as<std::string>();
}

Block get_genesis_block(std::string config_file) {
    Block genesis;
    YAML::Node config = YAML::LoadFile(GENESIS_FILE);

    //load header

    genesis.header.hash       = base58_decode_key(config["header"]["hash"].as<std::string>());
    genesis.header.prev_hash  = base58_decode_key(config["header"]["prev_hash"].as<std::string>());
    genesis.header.difficulty = config["header"]["difficulty"].as<uint32_t>();
    genesis.header.timestamp  = config["header"]["timestamp"].as<uint64_t>();
    genesis.header.id         = config["header"]["id"].as<uint32_t>();

    genesis.header.input.fingerprint.fill(0);
    genesis.header.input.public_key.fill(0);
    genesis.header.input.nonce = 0;

    // load transactions

    for(auto iter : config["transactions"]) {
        Transaction tx;
        tx.src  = base58_decode_key(iter["src"].as<std::string>());
        tx.dest = base58_decode_key(iter["dest"].as<std::string>());
        tx.amount = iter["amount"].as<uint32_t>();

        tx.signature.fill(0);
        tx.timestamp = 0;
        tx.id = 0;

        genesis.transactions.push_back(tx);
    }

    return genesis;
}
