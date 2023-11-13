#include <array>
#include <vector>
#include <cstdint>

typedef std::array<uint8_t, 32> Ed25519Key;
typedef std::array<uint8_t, 64> Ed25519Signature;

void gen_keys_ed25519(Ed25519Key &pub_key, Ed25519Key &priv_key);
Ed25519Signature sign_data_ed25519(Ed25519Key priv_key, std::vector<unsigned char> data);
bool verify_signature_ed25519(Ed25519Key pub_key, Ed25519Signature signature, std::vector<unsigned char> data);