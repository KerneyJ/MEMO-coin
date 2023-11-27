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
    //for every transaction in the transaction pool, if status is UNCONFIRMED, print transaction src,dest,signature,id,amount,timestamp,status
}
//prints the number of validators in the network.
void num_validators() {
    //metronome keeps track of the number of validators. Query the metronome.
}

//prints statistics on the network size, especially as it relates to proof of memory and proof of space resources.
void netspace() {
    //Metronome keeps track of the netspace. Query the metronome.
}

// (optional) prints the kth block in the blockchain.
void print_block(int k) {

}