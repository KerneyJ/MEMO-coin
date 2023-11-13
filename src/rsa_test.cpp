#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <string>
#include <vector>

typedef std::array<uint8_t, 32> Ed25519Key;
typedef std::array<uint8_t, 64> Ed25519Signature;

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

Ed25519Signature sign_data_ed25519(Ed25519Key priv_key, std::vector<unsigned char> data)
{
    Ed25519Signature signature;
    size_t sig_len = signature.size();

    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    EVP_PKEY *ed_key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, priv_key.cbegin(), priv_key.size());

    EVP_DigestSignInit(md_ctx, NULL, NULL, NULL, ed_key);
    EVP_DigestSign(md_ctx, signature.data(), &sig_len, data.data(), data.size());

    EVP_MD_CTX_free(md_ctx);
    return signature;
}

bool verify_signature_ed25519(Ed25519Key pub_key, Ed25519Signature signature, std::vector<unsigned char> data) {
    bool is_valid;

    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    EVP_PKEY *ed_key = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub_key.cbegin(), pub_key.size());

    EVP_DigestVerifyInit(md_ctx, NULL, NULL, NULL, ed_key);
    is_valid = EVP_DigestVerify(md_ctx, signature.data(), signature.size(), data.data(), data.size());

    EVP_MD_CTX_free(md_ctx);
    return is_valid;
}

int main(int argc, char* argv[]) 
{
    Ed25519Key pub_key;
    Ed25519Key priv_key;
    Ed25519Signature signature;

    std::string message = "lets sign this dude!";
    std::string not_my_message = "i took her to the penthouse then i 😨 it.";
    // std::string message = "this is my very sensitive data. please do not tell anybody. Bees are winged insects closely related to wasps and ants, known for their roles in pollination and, in the case of the best-known bee species, the western honey bee, for producing honey. Bees are a monophyletic lineage within the superfamily Apoidea. They are currently considered a clade, called Anthophila[citation needed]. There are over 20,000 known species of bees in seven recognized biological families.[1][2][3] Some species – including honey bees, bumblebees, and stingless bees – live socially in colonies while most species (>90%) – including mason bees, carpenter bees, leafcutter bees, and sweat bees – are solitary.";

    std::vector<uint8_t> data;
    data.resize(message.size());
    memcpy(data.data(), message.c_str(), message.size());

    std::vector<uint8_t> not_my_data;
    not_my_data.resize(not_my_message.size());
    memcpy(not_my_data.data(), not_my_message.c_str(), not_my_message.size());

    gen_keys_ed25519(pub_key, priv_key);

    printf("public key (%lu bytes):\n", pub_key.size());
    for(int i = 0; i < pub_key.size(); i++)
        printf("%02x", pub_key[i]);
    printf("\n");

    printf("private key (%lu bytes):\n", priv_key.size());
    for(int i = 0; i < priv_key.size(); i++)
        printf("%02x", priv_key[i]);
    printf("\n");

    signature = sign_data_ed25519(priv_key, data);

    printf("signed buffer (%lu bytes):\n", signature.size());
    for(int i = 0; i < signature.size(); i++)
        printf("%02x", signature[i]);
    printf("\n");

    printf("Should be valid: %d\n", verify_signature_ed25519(pub_key, signature, data));
    printf("Should not be valid: %d\n", verify_signature_ed25519(pub_key, signature, not_my_data));
    
    return 0;
}