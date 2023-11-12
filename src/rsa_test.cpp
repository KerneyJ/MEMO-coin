#include <array>
#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

void generateKeys() {
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

    std::array<uint8_t, 128> pub_key;
    std::array<uint8_t, 128> priv_key;
    size_t pub_size = pub_key.size();
    size_t priv_size = priv_key.size();

    EVP_PKEY_get_raw_public_key(pkey, pub_key.data(), &pub_size);
    EVP_PKEY_get_raw_private_key(pkey, priv_key.data(), &priv_size);

    printf("public key (%lu bytes):\n", pub_size);
    for(int i = 0; i < pub_size; i++)
        printf("%02x ", pub_key[i]);
    printf("\n");

    printf("private key (%lu bytes):\n", priv_size);
    for(int i = 0; i < priv_size; i++)
        printf("%02x ", priv_key[i]);
    printf("\n");

    EVP_PKEY_free(pkey);
}

int main(int argc, char* argv[]) 
{
    generateKeys();
    return 0;
}