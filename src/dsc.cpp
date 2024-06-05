#include <cstddef>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>
#include <zmq.hpp>

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
#include "utils.hpp"
#include "monitor.hpp"

int cl_wallet_help() {
    printf("Help menu for Wallet, supported commands:\n");
    printf("./dsc wallet help\n");
    printf("./dsc wallet create <config file> <save key file>\n");
    printf("./dsc wallet key <config file> <key file>\n");
    printf("./dsc wallet balance <config file> <key file>\n");
    printf("./dsc wallet send <config file> <key file> <amount> <address>\n");
    printf("./dsc wallet send multi <config file> <key file> <amount> <address> <count>\n");
    printf("./dsc wallet transaction <config file> <key file> <ID> \n");

    return 0;
}

int cl_wallet_create(std::string config_file, std::string key_file) {
    printf("Creating wallet...\n");

    Wallet wallet = create_wallet();
    store_wallet(wallet, config_file, key_file);
    display_wallet(wallet);

    return 0;
}

int cl_wallet_key(std::string config_file, std::string key_file) {
    Wallet wallet;

    printf("Loading wallet...\n");
    load_wallet(wallet, config_file, key_file);
    display_wallet(wallet);

    return 0;
}

int cl_wallet_balance(std::string config_file, std::string key_file) {
    std::string blockchain_node = get_blockchain_address(config_file);
    uint64_t balance = query_balance(blockchain_node, config_file, key_file);
    printf("Current balance: %lu\n", balance);
    return 0;
}

int cl_wallet_send(std::string config_file, std::string key_file, std::string arg_amount, std::string arg_address) {
    Transaction tx;
    Wallet wallet;
    Ed25519Key dest;
    std::string address;
    int amount, status;

    dest = base58_decode_key(arg_address);
    amount = std::stoi(arg_amount);
    load_wallet(wallet, config_file, key_file);

    printf("Creating transaction...\n");
    tx = create_transaction(wallet, dest, amount, get_tx_id(config_file));
    display_transaction(tx);

    address = get_tx_pool_address(config_file);
    status = submit_transaction(tx, address);

    if(status < 0) {
        printf("Transaction failed to post.\n");
        return -1;
    }

    printf("Transaction submitted successfully!\n");

    return 0;
}

int cl_wallet_send_multi(std::string config_file, std::string key_file, std::string arg_amount, std::string arg_address, std::string arg_count) {
    Transaction tx;
    Wallet wallet;
    Ed25519Key dest;
    std::string address;
    int amount, status, count;

    dest = base58_decode_key(arg_address);
    amount = std::stoi(arg_amount);
    count = std::stoi(arg_count);

    load_wallet(wallet, config_file, key_file);
    address = get_tx_pool_address(config_file);
    uint32_t id = get_tx_id(config_file);

    printf("Submitting %d transactions...\n", count);
    uint64_t start = get_timestamp();

    for(int i = 0; i < count; i++) {
        tx = create_transaction(wallet, dest, amount, id + i);
        status = submit_transaction(tx, address);

        if(status < 0) {
            printf("Transaction %d failed to post.\n", i);
            return -1;
        }
    }

    printf("Transactions submitted successfully! [%lu ms]\n", get_timestamp() - start);
    set_tx_id(id + count, config_file);

    return 0;
}

int cl_wallet_transaction(std::string config_file, std::string key_file, std::string arg_id) {
    std::string address = get_tx_pool_address(config_file);
    uint64_t id = std::stoi(arg_id);
    auto status = query_transaction(config_file, key_file, id, address);

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
        return cl_wallet_create(args[1], args[2]);

    if (args[0] == "key" && args.size() >= 3)
        return cl_wallet_key(args[1], args[2]);

    if (args[0] == "balance")
        return cl_wallet_balance(args[1], args[2]);

    if (args[0] == "send" && args.size() >= 7 && args[1] == "multi")
        return cl_wallet_send_multi(args[2], args[3], args[4], args[5], args[6]);

    if (args[0] == "send" && args.size() >= 5)
        return cl_wallet_send(args[1], args[2], args[3], args[4]);

    if (args[0] == "transaction" && args.size() >= 4)
        return cl_wallet_transaction(args[1], args[2], args[3]);

    printf("Please make sure you provided the correct command with all of its arguments.\n");
    return -1;
}

int run_pool(std::vector<std::string> args) {
    printf("Starting transaction pool.\n");
    if(args.size() < 1){
        printf("Not enough arguments for blockchain\n");
        printf("./bin/dsc pool <config file>\n");
        return -1;
    }
    std::string config_file = args[0];
    std::string pool_address = get_tx_pool_address(config_file);
    std::string bc_address = get_blockchain_address(config_file);
    TxPool tx_pool = TxPool(bc_address);
    tx_pool.start(pool_address);
    return 0;
}

int run_validator(std::vector<std::string> args) {
    printf("Starting validator.\n");
    Wallet wallet;
    if(args.size() < 2){
        printf("Not enough arguments for blockchain\n");
        printf("./bin/dsc blockchain <config file> <private key file>\n");
        return -1;
    }
    std::string config_file = args[0];
    std::string key_file = args[1];
    std::string val_address = get_validator_address(config_file);
    std::string bc_address = get_blockchain_address(config_file);
    std::string met_address = get_metronome_address(config_file);
    std::string pool_address = get_tx_pool_address(config_file);
    std::string consensus_type = get_consensus_method(config_file);
    load_wallet(wallet, config_file, key_file);

    IConsensusModel* model;

    if(consensus_type == "pow") {
        model = new ProofOfWork();
    } else if (consensus_type == "pom") {
        UUID fingerprint;
        uint32_t memory;
        ProofOfMemory* pom;

        uuid_generate(fingerprint.data());
        set_validator_fingerprint(fingerprint, args[1]);
        memory = get_validator_memory(args[1]);

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
    if(args.size() < 1){
        printf("Not enough arguments for blockchain\n");
        printf("./bin/dsc metronome <config file>\n");
        return -1;
    }
    std::string config_file = args[0];
    std::string blockchain = get_blockchain_address(config_file);
    std::string address = get_metronome_address(config_file);
    std::string consensus_type = get_consensus_method(config_file);
    bool sync_chain = get_sync_chain(config_file);
    Metronome metronome = Metronome(blockchain, consensus_type);
    metronome.start(address);
    return 0;
}

int run_blockchain(std::vector<std::string> args) {
    // TODO make a config argument to make the blockchain start by syncing with a peer node
    printf("Starting blockchain.\n");
    if(args.size() < 1){
        printf("Not enough arguments for blockchain\n");
        printf("./bin/dsc blockchain <config file>\n");
        return -1;
    }
    std::string config_file = args[0];

    std::string address = get_blockchain_address(config_file);
    std::string txpaddr = get_tx_pool_address(config_file);
    std::string metroaddr = get_metronome_address(config_file);
    std::string consensus_type = get_consensus_method(config_file);
    bool sync_chain = get_sync_chain(config_file);
    BlockChain blockchain = BlockChain(txpaddr, metroaddr, consensus_type, sync_chain, config_file);
    blockchain.start(address);
    return 0;
}

//Prints out all the monitor stats when called. Could be modified to print stats at regular intervals in a loop.
int run_monitor(std::vector<std::string> args) {
    printf("Starting monitor\n");
    print_monitor_stats(args[1]);
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
