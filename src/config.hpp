#include <string>

#include "wallet.hpp"
#include "address_list.hpp"

#define CONFIG_FILE "dsc-config.yaml"
#define PRIVATE_KEY_FILE "dsc-key.yaml"

// Wallet config

bool load_wallet(Wallet& wallet);
bool store_wallet(Wallet& wallet);
uint64_t get_tx_id();

// Trancastion pool config

std::string get_tx_pools_threads();
std::string get_tx_pools_address();

// Validator config

std::string get_validator_address();
std::string get_validator_threads();
std::string get_consensus_method();

// Metronome config

std::string get_blockchain_address();
std::string get_blockchain_threads();

// Blockchain config

std::string get_metronome_address();
std::string get_metronome_threads();
