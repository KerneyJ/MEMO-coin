#include <cstddef>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>
#include <zmq.h>

extern "C" {
    #include "../external/base58/base58.h"
}
#include "defs.hpp"
#include "wallet.hpp"
#include "transaction.hpp"
#include "keys.hpp"
#include "blockchain.hpp"
#include "config.hpp"
#include "transaction.hpp"
#include "tx_pool.hpp"
#include "validator.hpp"
#include "metronome.hpp"
#include "consensus.hpp"
#include "pom.hpp"
#include "pow.hpp"
#include "monitor.hpp"

int cl_wallet_help() {
    printf("Help menu for Wallet, supported commands:\n");
    printf("./dsc wallet help\n");
    printf("./dsc wallet create\n");
    printf("./dsc wallet key\n");
    printf("./dsc wallet balance\n");
    printf("./dsc wallet send <amount> <address>\n");
    printf("./dsc wallet transaction <ID>\n");

    return 0;
}

int cl_wallet_create() {
    printf("Creating wallet...\n");

    Wallet wallet = create_wallet();
    store_wallet(wallet);
    display_wallet(wallet);

    return 0;
}

int cl_wallet_key() {
    Wallet wallet;

    printf("Loading wallet...\n");
    load_wallet(wallet);
    display_wallet(wallet);

    return 0;
}

int cl_wallet_balance() {
    std::string blockchain_node = get_blockchain_address();
    uint64_t balance = query_balance(blockchain_node);
    printf("Current balance: %lu\n", balance);
    return 0;
}

int cl_wallet_send(std::string arg_amount, std::string arg_address) {
    Transaction tx;
    Wallet wallet;
    Ed25519Key dest;
    std::string address;
    int amount, status;
    
    dest = base58_decode_key(arg_address);
    amount = std::stoi(arg_amount);
    load_wallet(wallet);

    printf("Creating transaction...\n");
    tx = create_transaction(wallet, dest, amount, get_tx_id());
    display_transaction(tx);

    address = get_tx_pool_address();
    status = submit_transaction(tx, address);

    if(status < 0) {
        printf("Transaction failed to post.\n");
        return -1;
    } 

    printf("Transaction submitted successfully!\n");

    return 0;
}

int cl_wallet_transaction(std::string arg_id) {
    std::string address = get_tx_pool_address();
    uint64_t id = std::stoi(arg_id);
    auto status = query_transaction(id, address);

    printf("Transaction %lu status [%s]\n", id, 
        (status == Transaction::CONFIRMED)   ? "CONFIRMED" :
        (status == Transaction::UNCONFIRMED) ? "UNCONFIRMED" :
        (status == Transaction::SUBMITTED)   ? "SUBMITTED" :
        (status == Transaction::UNKNOWN)     ? "UNKNOWN" : "NULL");

    return 0;
}

int run_wallet(std::vector<std::string> args) {
    if (args.empty() || args[0] == "help")
        return cl_wallet_help();

    if (args[0] == "create")
        return cl_wallet_create();

    if (args[0] == "key")
        return cl_wallet_key();

    if (args[0] == "balance")
        return cl_wallet_balance();

    if (args[0] == "send" && args.size() >= 3)
        return cl_wallet_send(args[1], args[2]);

    if (args[0] == "transaction" && args.size() >= 2)
        return cl_wallet_transaction(args[1]);

    printf("Please make sure you provided the correct command with all of its arguments.\n");
    return -1;
}

int run_pool(std::vector<std::string> args) {
    printf("Starting transaction pool.\n");
    std::string pool_address = get_tx_pool_address();
    std::string bc_address = get_blockchain_address();
    TxPool tx_pool = TxPool(bc_address);
    tx_pool.start(pool_address);
    return 0;
}

int run_validator(std::vector<std::string> args) {
    printf("Starting validator.\n");
    Wallet wallet;

    std::string val_address = get_validator_address();
    std::string bc_address = get_blockchain_address();
    std::string met_address = get_metronome_address();
    std::string pool_address = get_tx_pool_address();
    std::string consensus_type = get_consensus_method();
    load_wallet(wallet);

    IConsensusModel* model;

    if(consensus_type == "pow") {
        model = new ProofOfWork();
    } else if (consensus_type == "pom") {
        UUID fingerprint;
        uint32_t memory;
        ProofOfMemory* pom;

        uuid_generate(fingerprint.data());
        set_validator_fingerprint(fingerprint);
        memory = get_validator_memory();

        pom = new ProofOfMemory(wallet, fingerprint);
        pom->gen_hashes(memory);

        model = pom;
    } else {
        printf("Unknown consensus method.\n");
        return -1;
    }

    Validator validator = Validator(bc_address, met_address, pool_address, model, wallet);
    validator.start(val_address);

    return -1;
}

int run_metronome(std::vector<std::string> args) {
    printf("Starting metronome.\n");
    std::string blockchain = get_blockchain_address();
    std::string address = get_metronome_address();
    Metronome metronome = Metronome(blockchain);
    metronome.start(address);
    return 0;
}

int run_blockchain(std::vector<std::string> args) {
    printf("Starting blockchain.\n");
    std::string address = get_blockchain_address();
    std::string txpaddr = get_tx_pool_address();
    BlockChain blockchain = BlockChain(txpaddr);
    blockchain.start(address);
    return 0;
}

//Prints out all the monitor stats when called. Could be modified to print stats at regular intervals in a loop.
int run_monitor(std::vector<std::string> args) {
    printf("Starting monitor\n");
    print_monitor_stats();
    printf("Monitor stats complete.");
    return 0;
}

int main(int argc, char** argv) {
    printf("DSC: DataSys Coin Blockchain v1.0\n");

    if(argc < 2) {
        printf("Please provide a component to run.\n");
        return -1;
    }

    std::string command(argv[1]);
    std::vector<std::string> args;

    for(int i = 2; i < argc; i++)
        args.push_back(argv[i]);

    if(command == "wallet")
        return run_wallet(args);
    
    if(command == "pool")
        return run_pool(args);

    if(command == "metronome")
        return run_metronome(args);

    if(command == "validator")
        return run_validator(args);

    if(command == "blockchain")
        return run_blockchain(args);

    if(command == "monitor")
        return run_monitor(args);
    printf("Could not find component to run.\n");
    return -1;
}
