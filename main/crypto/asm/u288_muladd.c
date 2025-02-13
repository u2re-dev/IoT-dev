#ifdef ENABLE_ECC_M
    static uint32_t u288_muladd(uint32_t z[9], uint32_t x, const uint32_t y[8])
    {
        uint32_t carry = 0;

    #define U288_MULADD_STEP(i) \
        do { \
            uint64_t prod = u32_muladd64(x, y[i], z[i], carry); \
            z[i] = (uint32_t) prod; \
            carry = (uint32_t) (prod >> 32); \
        } while( 0 )

    #if defined(MULADD64_SMALL)
        U288_MULADD_STEP(0);
        U288_MULADD_STEP(1);
        U288_MULADD_STEP(2);
        U288_MULADD_STEP(3);
        U288_MULADD_STEP(4);
        U288_MULADD_STEP(5);
        U288_MULADD_STEP(6);
        U288_MULADD_STEP(7);
    #else
        for (unsigned i = 0; i < 8; i++) {
            U288_MULADD_STEP(i);
        }
    #endif

        uint64_t sum = (uint64_t) z[8] + carry;
        z[8] = (uint32_t) sum;
        carry = (uint32_t) (sum >> 32);

        return carry;
    }
#endif
