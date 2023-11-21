#include "transaction.hpp"
#include "tx_pool.hpp"
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
    query_balance();
    return 0;
}

int cl_wallet_send(std::string arg_amount, std::string arg_address) {
    Transaction tx;
    Wallet wallet;
    Ed25519Key dest;
    int amount;
    
    dest = base58_decode(arg_address);
    amount = std::stoi(arg_amount);
    load_wallet(wallet);

    printf("Creating transaction...\n");
    tx = create_transaction(wallet, dest, amount, get_tx_id());
    display_transaction(tx);

    int status = submit_transaction(tx, TX_POOL_ADDR);

    if(status < 0) {
        printf("Transaction failed to post.\n");
        return -1;
    } 

    printf("Transaction submitted successfully!\n");

    return 0;
}

int cl_wallet_transaction(std::string arg_id) {
    uint64_t id = std::stoi(arg_id);
    auto status = query_transaction(id, TX_POOL_ADDR);

    printf("Transaction %lu status [%s]\n", id, 
        (status == Transaction::CONFIRMED)   ? "CONFIRMED" :
        (status == Transaction::UNCONFIRMED) ? "UNCONFIRMED" :
        (status == Transaction::SUBMITTED)   ? "SUBMITTED" :
        (status == Transaction::UNKNOWN)     ? "UNKNOWN" : "NULL");

    return 0;
}

int handle_wallet_command(std::vector<std::string> args) {
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

int handle_pool_command(std::vector<std::string> args) {
    TxPool tx_pool = TxPool();
    return 0;
}

int handle_blockchain(std::vector<std::string> args){
    BlockChain bc = BlockChain();
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
        return handle_wallet_command(args);
    
    if(command == "pool")
        return handle_pool_command(args);

<<<<<<< HEAD
    if(command == "blockchain")
        return handle_blockchain(args);
=======
    if(command == "metronome")
        return handle_pool_command(args);

    if(command == "validator")
        return handle_pool_command(args);
>>>>>>> 60e5fcf483f1b439bd1306fac63e0a8bfd8877bc

    printf("Could not find component to run.\n");
    return -1;
}
