#ifdef ENABLE_ECC_M
    static uint64_t u32_muladd64(uint32_t x, uint32_t y, uint32_t z, uint32_t t);

    #undef MULADD64_ASM
    #undef MULADD64_SMALL
    #if defined(__GNUC__) &&\
        defined(__ARM_ARCH) && __ARM_ARCH >= 6 && defined(__ARM_ARCH_PROFILE) && \
        ( __ARM_ARCH_PROFILE == 77 || __ARM_ARCH_PROFILE == 65 )

    #if defined(__ARM_FEATURE_DSP)

        static uint64_t u32_muladd64(uint32_t x, uint32_t y, uint32_t z, uint32_t t)
        {
            __asm__(
                "umaal   %[z], %[t], %[x], %[y]"
                : [z] "+l" (z), [t] "+l" (t)
                : [x] "l" (x), [y] "l" (y)
            );
            return ((uint64_t) t << 32) | z;
        }
        #define MULADD64_ASM
        #define MULADD64_SMALL

    #else

        static uint64_t u32_muladd64(uint32_t x, uint32_t y, uint32_t z, uint32_t t)
        {
            uint32_t tmp1, tmp2, tmp3;
            __asm__(
                ".syntax unified\n\t"
                "lsrs    %[u], %[x], #16\n\t"
                "lsrs    %[v], %[y], #16\n\t"
                "uxth    %[x], %[x]\n\t"
                "uxth    %[y], %[y]\n\t"
                "movs    %[w], %[v]\n\t"
                "muls    %[w], %[u]\n\t"
                "muls    %[v], %[x]\n\t"
                "muls    %[x], %[y]\n\t"
                "muls    %[y], %[u]\n\t"
                "lsls    %[u], %[y], #16\n\t"
                "lsrs    %[y], %[y], #16\n\t"
                "adds    %[x], %[u]\n\t"
                "adcs    %[y], %[w]\n\t"
                "lsls    %[u], %[v], #16\n\t"
                "lsrs    %[v], %[v], #16\n\t"
                "adds    %[x], %[u]\n\t"
                "adcs    %[y], %[v]\n\t"
                : [x] "+l" (x), [y] "+l" (y), [u] "=&l" (tmp1), [v] "=&l" (tmp2), [w] "=&l" (tmp3)
                :
                : "cc"
            );
            (void) tmp1;
            (void) tmp2;
            (void) tmp3;

            __asm__(
                ".syntax unified\n\t"
                "movs    %[u], #0\n\t"
                "adds    %[x], %[z]\n\t"
                "adcs    %[y], %[u]\n\t"
                "adds    %[x], %[t]\n\t"
                "adcs    %[y], %[u]\n\t"
                : [x] "+l" (x), [y] "+l" (y), [u] "=&l" (tmp1)
                : [z] "l" (z), [t] "l" (t)
                : "cc"
            );
            (void) tmp1;

            return ((uint64_t) y << 32) | x;
        }
        #define MULADD64_ASM
        #endif
    #endif

    #if !defined(MULADD64_ASM)
        #if defined(MUL64_IS_CONSTANT_TIME)
        static uint64_t u32_muladd64(uint32_t x, uint32_t y, uint32_t z, uint32_t t)
        {
            return (uint64_t) x * y + z + t;
        }
        #define MULADD64_SMALL
        #else
        static uint64_t u32_muladd64(uint32_t x, uint32_t y, uint32_t z, uint32_t t)
        {
            const uint16_t xl = (uint16_t) x;
            const uint16_t yl = (uint16_t) y;
            const uint16_t xh = x >> 16;
            const uint16_t yh = y >> 16;

            const uint32_t lo = (uint32_t) xl * yl;
            const uint32_t m1 = (uint32_t) xh * yl;
            const uint32_t m2 = (uint32_t) xl * yh;
            const uint32_t hi = (uint32_t) xh * yh;

            uint64_t acc = lo + ((uint64_t) (hi + (m1 >> 16) + (m2 >> 16)) << 32);
            acc += m1 << 16;
            acc += m2 << 16;
            acc += z;
            acc += t;

            return acc;
        }
        #endif
    #endif

#endif
