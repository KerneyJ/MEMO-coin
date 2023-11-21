#include <ctime>
#include <string>
#include <chrono>

#include "transaction.hpp"
#include "keys.hpp"
#include "wallet.hpp"
#include "utils.hpp"

void sign_transaction(Ed25519Key priv_key, Transaction &tx) {
    tx.signature.fill(0);
    tx.signature = sign_data_ed25519(priv_key, (uint8_t*) &tx, sizeof(Transaction));
}

bool verify_transaction_signature(Ed25519Key pub_key, Transaction tx) {
    Ed25519Signature signature = tx.signature;
    tx.signature.fill(0);
    return verify_signature_ed25519(pub_key, signature, (uint8_t*) &tx, sizeof(Transaction));
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
}