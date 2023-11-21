#include <cstring>
#include <ctime>
#include <string>
#include <chrono>
#include <system_error>
#include <zmq.h>

#include "transaction.hpp"
#include "keys.hpp"
#include "wallet.hpp"
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

void display_transaction(Transaction tx) {
    std::string src = base58_encode(tx.src);
    std::string dest = base58_encode(tx.dest);

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

    auto req_buf = serialize_message(tx, POST_TX);
    zmq_send (requester, req_buf.data(), req_buf.size(), 0);

    ReceiveBuffer res_buf;
    zmq_recv (requester, res_buf.data(), res_buf.size(), 0);
    auto response = deserialize_message<NullMessage>(res_buf);

    zmq_close (requester);
    zmq_ctx_destroy (context);

    return (response.type == STATUS_GOOD) ? 0 : -1;
}


    // std::vector<uint8_t> req_bytes;
    // alpaca::serialize<OPTIONS>(tx, req_bytes);

    // std::array<uint8_t, 1000> res_bytes;
    // memcpy(res_bytes.data(), req_bytes.data(), req_bytes.size());
    // std::error_code ec;
    // Transaction detx = alpaca::deserialize<OPTIONS, Transaction>(res_bytes, ec);
    // display_transaction(detx);
    // printf("errors: %s\n", ec.message().c_str());
    // printf("signature: %d\n", verify);