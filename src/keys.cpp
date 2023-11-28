#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <stdio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <string>
#include <vector>

extern "C" {
    #include "../external/base58/base58.h"
}
#include "keys.hpp"

void gen_keys_ed25519(Ed25519Key &pub_key, Ed25519Key &priv_key) {
    size_t pub_size = pub_key.size();
    size_t priv_size = priv_key.size();

    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);
    
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_keygen(pctx, &pkey);
    EVP_PKEY_CTX_free(pctx);

    if(pkey==NULL){
        fprintf(stderr,"error: rsa gen\n");
        ERR_print_errors_fp(stderr);
        return;
    }

    EVP_PKEY_get_raw_private_key(pkey, priv_key.data(), &priv_size);
    EVP_PKEY_get_raw_public_key(pkey, pub_key.data(), &pub_size);

    EVP_PKEY_free(pkey);
}

Ed25519Signature sign_data_ed25519(Ed25519Key priv_key, uint8_t* data, size_t len)
{
    Ed25519Signature signature;
    size_t sig_len = signature.size();

    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    EVP_PKEY *ed_key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, priv_key.cbegin(), priv_key.size());

    EVP_DigestSignInit(md_ctx, NULL, NULL, NULL, ed_key);
    EVP_DigestSign(md_ctx, signature.data(), &sig_len, data, len);

    EVP_MD_CTX_free(md_ctx);
    return signature;
}

bool verify_signature_ed25519(Ed25519Key pub_key, Ed25519Signature signature, uint8_t* data, size_t len) {
    bool is_valid;

    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    EVP_PKEY *ed_key = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub_key.cbegin(), pub_key.size());

    EVP_DigestVerifyInit(md_ctx, NULL, NULL, NULL, ed_key);
    is_valid = EVP_DigestVerify(md_ctx, signature.data(), signature.size(), data, len);

    EVP_MD_CTX_free(md_ctx);
    return is_valid;
}

std::string base58_encode_key(Ed25519Key key) {
    size_t len = 128;
    char encoded[128];

    if(!b58enc(encoded, &len, key.data(), key.size()))
        throw std::runtime_error("Failed to encode key.");

    return std::string(encoded);
}

Ed25519Key base58_decode_key(std::string str) {
    Ed25519Key key;
    size_t len = key.size();

    if(!b58tobin(key.data(), &len, str.c_str())) 
        throw std::runtime_error("Failed to encode key.");

    return key;
}

std::string base58_encode_uuid(UUID uuid) {
    size_t len = 128;
    char encoded[128];

    if(!b58enc(encoded, &len, uuid.data(), uuid.size()))
        throw std::runtime_error("Failed to encode uuid.");

    return std::string(encoded);
}

UUID base58_decode_uuid(std::string str) {
    UUID uuid;
    size_t len = uuid.size();

    if(!b58tobin(uuid.data(), &len, str.c_str())) 
        throw std::runtime_error("Failed to encode uuid.");

    return uuid;
}