/* 
Commands for getting info about the blockchain's state.
*/

#include "defs.hpp"
#include "transaction.hpp"
#include "wallet.hpp"
#include "keys.hpp"
#include "messages.hpp"


//prints the contents of the last block of the blockchain.
void last_block() {

}


//prints info about the unconfirmed transactions in the transaction pools.
void unconf_trans() {
    //makes a COUNT_UNCONFIRMED_TX request to the tx_pool.
    //in tx_pool.cpp, the tx pool counts the returns the number of transactions in the queue and replies with that number.

    void* context = zmq_ctx_new();
    void* requester = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(requester, tx_pool.c_str());

    Message<uint32_t> response;
    request_response(requester, QUERY_TX_COUNT, response);

    zmq_close(requester);
    zmq_ctx_destroy(context);
    printf("size of transation queue: %d transactions in queue", response.data);
    return;
}
//prints the number of validators in the network.
void num_validators() {
    //metronome keeps track of the number of validators. Query the metronome.
}
    
//prints the last block header
void last_block() {

}
//prints the number of wallet addresses
void num_wallets() {

}

//prints the total number of coins in circulation
void num_coins() {

}

// prints the number of hashes per second
void hashes_per_second() {

}

//prints the total number of hashes stored
void hashes_stored() {

}