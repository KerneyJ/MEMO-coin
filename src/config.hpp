#include <string>

#include "block.hpp"
#include "wallet.hpp"

#define CONFIG_FILE "dsc-config.yaml"
#define PRIVATE_KEY_FILE "dsc-key.yaml"
#define GENESIS_FILE "genesis.yaml"

// global config
bool get_sync_chain(std::string config_file);

// Wallet config

bool load_wallet(Wallet& wallet, std::string config_file, std::string key_file);
bool store_wallet(Wallet& wallet, std::string config_file, std::string key_file);
uint64_t get_tx_id(std::string config_file);
void set_tx_id(uint64_t id, std::string config_file);

// Trancastion pool config

std::string get_tx_pool_threads(std::string config_file);
std::string get_tx_pool_address(std::string config_file);

// Validator config

std::string get_validator_address(std::string config_file);
std::string get_validator_threads(std::string config_file);
std::string get_consensus_method(std::string config_file);
uint32_t get_validator_memory(std::string config_file);
void set_validator_fingerprint(UUID, std::string config_file);

// Metronome config

std::string get_metronome_address(std::string config_file);
std::string get_metronome_threads(std::string config_file);

// Blockchain config

std::string get_blockchain_address(std::string config_file);
std::string get_blockchain_threads(std::string config_file);
Block get_genesis_block(std::string config_file);
