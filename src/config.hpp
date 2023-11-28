#include <string>

#include "block.hpp"
#include "wallet.hpp"

#define CONFIG_FILE "dsc-config.yaml"
#define PRIVATE_KEY_FILE "dsc-key.yaml"
#define GENESIS_FILE "genesis.yaml"

// Wallet config

bool load_wallet(Wallet& wallet);
bool store_wallet(Wallet& wallet);
uint64_t get_tx_id();

// Trancastion pool config

std::string get_tx_pool_threads();
std::string get_tx_pool_address();

// Validator config

std::string get_validator_address();
std::string get_validator_threads();
std::string get_consensus_method();
uint32_t get_validator_memory();
void set_validator_fingerprint(UUID);

// Metronome config

std::string get_metronome_address();
std::string get_metronome_threads();

// Blockchain config

std::string get_blockchain_address();
std::string get_blockchain_threads();
Block get_genesis_block();
