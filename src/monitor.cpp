/* 
Commands for getting info about the blockchain's state.
*/

#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "keys.hpp"
#include "messages.hpp"
#include <zmq.h>
#include "config.hpp"
#include "utils.hpp"
#include "block.hpp"


//prints the header of the last block of the blockchain.
void last_block() {
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    std::string blockchain_address = get_blockchain_address();
    zmq_connect(requester, blockchain_address.c_str());

    Message<Block> response;
    Block b = response.data;
    request_response(requester, QUERY_LAST_BLOCK, response);
    zmq_close(requester);
    zmq_ctx_destroy(context);

    display_block(b);
    printf("\n");
    return;
}


//prints info about the unconfirmed transactions in the transaction pools.
//TODO: should show separate values for the number of unconfirmed tx's and submitted tx's.
void num_trans() {
    //makes a COUNT_UNCONFIRMED_TX request to the tx_pool.
    //in tx_pool.cpp, the tx pool counts the returns the number of transactions in the queue and replies with that number.

    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    std::string tx_pool = get_tx_pool_address();
    zmq_connect(requester, tx_pool.c_str());

    Message<uint32_t> response;
    request_response(requester, QUERY_TX_COUNT, response);

    zmq_close(requester);
    zmq_ctx_destroy(context);
    printf("size of transation queue: %d transactions in queue", response.data);
    printf("\n");
    return;
}
//prints the number of validators in the network.
void num_validators() {
    //metronome keeps track of the number of validators. Query the metronome.
    printf("num_validators not implemented");
    printf("\n");
    return;
}
//prints the number of wallet addresses
void num_wallets() {
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    std::string blockchain_address = get_blockchain_address();
    zmq_connect(requester, blockchain_address.c_str());

    Message<int> response;
    request_response(requester, QUERY_NUM_ADDRS, response);
    int num_addresses = response.data;
    printf("Number of wallets on blockchain: %d\n", num_addresses);
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return;
}

//prints the total number of coins in circulation by adding up the value of all the accounts in the ledger.
void num_coins() {
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    std::string blockchain_address = get_blockchain_address();
    zmq_connect(requester, blockchain_address.c_str());

    Message<int> response;
    request_response(requester, QUERY_COINS, response);
    int coin_count = response.data;
    printf("Net total number of coins in blockchain: %d coins.\n", coin_count);
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return;
}

// prints the number of hashes per second
void hashes_per_second() {
    printf("hashes_per_second not implemented");
    printf("\n");
    return;
}

//prints the total number of hashes stored
void hashes_stored() {
    printf("hashes_stored not implemented");
    printf("\n");
    return;
}

//Run all the monitor stats
void print_monitor_stats() {
    printf("\ttimestamp: %lu\n", get_timestamp());
    last_block();
    num_trans();
    num_validators();
    num_wallets();
    num_coins();
    hashes_per_second();
    hashes_stored();
    printf("\tEnd of monitor stats. timestamp: %lu\n", get_timestamp());
    return;
}