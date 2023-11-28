#include <array>
#include <functional>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include "defs.hpp"

#pragma once

struct Ed25519KeyHash {
    std::size_t operator () (Ed25519Key const &v) const
    {
        std::hash<uint64_t> hasher;
        uint64_t* arr = (uint64_t*) v.data();

        auto h1 = hasher(v[0]);
        auto h2 = hasher(v[1]);
        auto h3 = hasher(v[2]);
        auto h4 = hasher(v[3]);

        return h1 ^ h2 ^ h3 ^ h4;
    }
};

void gen_keys_ed25519(Ed25519Key &pub_key, Ed25519Key &priv_key);
Ed25519Signature sign_data_ed25519(Ed25519Key priv_key, uint8_t* data, size_t len);
bool verify_signature_ed25519(Ed25519Key pub_key, Ed25519Signature signature, uint8_t* data, size_t len);

std::string base58_encode_key(Ed25519Key key);
Ed25519Key base58_decode_key(std::string str);

std::string base58_encode_uuid(UUID uuid);
UUID base58_decode_uuid(std::string str);