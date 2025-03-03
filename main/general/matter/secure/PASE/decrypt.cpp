#ifndef D06B583A_09D8_4619_B336_2BFFFD2BB5E2
#define D06B583A_09D8_4619_B336_2BFFFD2BB5E2

//
#include "../PASE.hpp"

//
SessionKeys& PASE::makeSessionKeys() {
    auto info = hex::s2b("SessionKeys");
    auto keys = crypto::hkdf_len(hkdf.Ke, info);

    // TODO: better interpret code
    return (channel.setSessionKeys(SessionKeys {
        *reinterpret_cast<intx::uint128*>(keys->data()),
        *reinterpret_cast<intx::uint128*>(keys->data() + 16),
        *reinterpret_cast<intx::uint128*>(keys->data() + 32)
    }));
}

//
#endif
