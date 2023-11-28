#include <cstring>
#include <ctime>
#include <string>
#include <chrono>
#include <system_error>
#include <zmq.h>

#include "transaction.hpp"
#include "keys.hpp"
#include "wallet.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "messages.hpp"

void sign_transaction(Ed25519Key priv_key, Transaction &tx) {
    tx.signature.fill(0);
    tx.signature = sign_data_ed25519(priv_key, (uint8_t*) &tx, sizeof(Transaction));
}

bool verify_transaction_signature(Transaction tx) {
    Ed25519Signature signature = tx.signature;
    tx.signature.fill(0);
    int status = verify_signature_ed25519(tx.src, signature, (uint8_t*) &tx, sizeof(Transaction));
    tx.signature = signature;
    return status;
}

Transaction create_transaction(Wallet wallet, Ed25519Key dest, uint32_t amount, uint64_t id) {
    Transaction tx;

    tx.src = wallet.pub_key;
    tx.dest = dest;
    tx.amount = amount;
    tx.id = id;
    tx.timestamp = get_timestamp();
    
    sign_transaction(wallet.priv_key, tx);

    return tx;
}

Transaction create_transaction(Wallet wallet) {
    Transaction reward = {
        .src = {},
        .dest = wallet.pub_key,
        .signature = {},
        .id = 0,
        .amount = MINER_REWARD,
        .timestamp = get_timestamp(),
    };

    reward.src.fill(0);
    reward.signature.fill(0);

    return reward;
}

void display_transaction(Transaction tx) {
    std::string src = base58_encode_key(tx.src);
    std::string dest = base58_encode_key(tx.dest);

    printf("Transaction:\n");
    printf("\tsrc: %s\n", src.c_str());
    printf("\tdest: %s\n", dest.c_str());
    printf("\tamount: %d\n", tx.amount);
    printf("\tid: %lu\n", tx.id);
    printf("\ttimestamp: %lu\n", tx.timestamp);
    printf("\tsignature: ");
    for(auto byte : tx.signature)
        printf("%02x", byte);
    printf("\n");
}

int submit_transaction(Transaction tx, std::string tx_pool) {
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);

    zmq_connect (requester, tx_pool.c_str());

    Message<NullMessage> response;
    request_response(requester, tx, POST_TX, response);

    zmq_close (requester);
    zmq_ctx_destroy (context);

    return (response.type == STATUS_GOOD) ? 0 : -1;
}

Transaction::Status query_transaction(uint64_t id, std::string tx_pool) {
    ReceiveBuffer res_buf;
    Wallet wallet;

    load_wallet(wallet);
    std::pair<Ed25519Key, uint64_t> tx_key = { wallet.pub_key, id };

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);

    zmq_connect (requester, tx_pool.c_str());

    Message<Transaction::Status> response;
    request_response(requester, tx_key, QUERY_TX_STATUS, response);

    zmq_close (requester);
    zmq_ctx_destroy (context);

    return response.data;
}