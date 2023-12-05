/* 
Commands for getting info about the blockchain's state.
*/

#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "keys.hpp"
#include "messages.hpp"
#include <cstdint>
#include <zmq.hpp>
#include "config.hpp"
#include "utils.hpp"
#include "block.hpp"


//prints the header of the last block of the blockchain.
void last_block() {
    zmq::context_t context;
    zmq::socket_t requester(context, ZMQ_REQ);
    std::string blockchain_address = get_blockchain_address();
    requester.connect(blockchain_address);

    send_message(requester, QUERY_LAST_BLOCK);
    auto response = recv_message<BlockHeader>(requester);

    display_block_header(response.data);
    printf("\n");
}


//prints info about the unconfirmed transactions in the transaction pools.
void num_trans() {
    //makes a COUNT_UNCONFIRMED_TX request to the tx_pool.
    //in tx_pool.cpp, the tx pool counts the returns the number of transactions in the queue and replies with that number.

    zmq::context_t context;
    zmq::socket_t requester(context, ZMQ_REQ);
    std::string tx_pool = get_tx_pool_address();
    requester.connect(tx_pool);

    send_message(requester, QUERY_TX_COUNT);
    auto response = recv_message<uint32_t>(requester);

    printf("Size of transation queue: %d\n", response.data);
    printf("\n");
}
//prints the number of validators in the network. Number is reset at beginning of each block.
//Validator registers with metronome when it attempts to mine the block.
void num_validators() {
    //metronome keeps track of the number of validators. Query the metronome.
    zmq::context_t context;
    zmq::socket_t requester(context, ZMQ_REQ);
    std::string metronome_address = get_metronome_address();
    requester.connect(metronome_address);

    send_message(requester, QUERY_NUM_VALIDATORS);
    auto response = recv_message<uint32_t>(requester);
    
    printf("Number of validators: %d\n", response.data);
}
//prints the number of wallet addresses
void num_wallets() {
    zmq::context_t context;
    zmq::socket_t requester(context, ZMQ_REQ);
    std::string blockchain_address = get_blockchain_address();
    requester.connect(blockchain_address);

    send_message(requester, QUERY_NUM_ADDRS);
    auto response = recv_message<uint32_t>(requester);
    
    printf("Number of wallets on blockchain: %d\n", response.data);
}

//prints the total number of coins in circulation by adding up the value of all the accounts in the ledger.
void num_coins() {
    zmq::context_t context;
    zmq::socket_t requester(context, ZMQ_REQ);
    std::string blockchain_address = get_blockchain_address();
    requester.connect(blockchain_address);

    send_message(requester, QUERY_COINS);
    auto response = recv_message<uint32_t>(requester);
    
    printf("Net total number of coins in blockchain: %d coins.\n", response.data);
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
