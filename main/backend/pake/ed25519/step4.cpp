#include "../std/utils.hpp"
#include <sodium.h>

//
int crypto_spake_step4(ServerState &st, SharedKeys &shared_keys, const unsigned char response3[crypto_spake_RESPONSE3BYTES])
{
    if(sodium_memcmp(response3, st.server_validator.data(), 32) != 0) {
        sodium_memzero(&st, sizeof(st));
        return -1;
    }
    std::memcpy(shared_keys.client_sk.data(), st.shared_keys.client_sk.data(), crypto_spake_SHAREDKEYBYTES);
    std::memcpy(shared_keys.server_sk.data(), st.shared_keys.server_sk.data(), crypto_spake_SHAREDKEYBYTES);
    sodium_memzero(&st, sizeof(st));
    return 0;
}
