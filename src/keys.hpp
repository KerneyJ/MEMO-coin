#include <array>
#include <string>
#include <vector>
#include <cstdint>

#include "defs.hpp"

void gen_keys_ed25519(Ed25519Key &pub_key, Ed25519Key &priv_key);
Ed25519Signature sign_data_ed25519(Ed25519Key priv_key, uint8_t* data, size_t len);
bool verify_signature_ed25519(Ed25519Key pub_key, Ed25519Signature signature, uint8_t* data, size_t len);

std::string base58_encode(Ed25519Key key);
Ed25519Key base58_dencode(std::string str);