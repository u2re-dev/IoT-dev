
// ? may be incorrect means
using byte_t = uint8_t[32];
using byte_2 = uint8_t[64];

// bit more correctly (may be)
struct byte_t { uint8_t _[32]; };
struct byte_2 { uint8_t _[64]; };


// ? may be incorrect means
using u256_t = uint32_t[8];

// bit more correctly (may be)
struct u256_t { uint32_t _[8]; };

// scalar type
using scalar_t = u256_t;

//
struct m256_t {
    u256_t x; //
    u256_t y; //
    //u256_t z; // by default may be identity constant
};

//
struct point_t {
    u256_t x; //
    u256_t y; //
    u256_t z; // may be derivative from m256_mod or by default may be identity constant
};

//
/*struct m256 {
    u256_t x;
    u256_t y;
    u256_t z;
};*/

// may also contains u256_t types
typedef struct {
    uint32_t m[8];
    uint32_t R2[8];
    uint32_t ni;
} m256_mod;





/*
 * The curve's b parameter in the Short Weierstrass equation
 *  y^2 = x^3 - 3*x + b
 * Compared to the standard, this is converted to the Montgomery domain.
 */

// u256_t type
static const uint32_t p256_b[8] = {
    0x29c4bddf, 0xd89cdf62, 0x78843090, 0xacf005cd,
    0xf7212ed6, 0xe5a220ab, 0x04874834, 0xdc30061d,
};


/*
 * The curve's conventional base point G.
 * Compared to the standard, coordinates converted to the Montgomery domain.
 */

// u256_t type
static const uint32_t p256_gx[8] = {
    0x18a9143c, 0x79e730d4, 0x5fedb601, 0x75ba95fc,
    0x77622510, 0x79fb732b, 0xa53755c6, 0x18905f76,
};

// u256_t type
static const uint32_t p256_gy[8] = {
    0xce95560a, 0xddf25357, 0xba19e45c, 0x8b4ab8e4,
    0xdd21f325, 0xd2e88688, 0x25885d85, 0x8571ff18,
};



// from original: data for Montgomery operations modulo the curve's p
// may be, base point (or related)
static const m256_mod p256_p = {
    {
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
        0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF,
    },
    {
        0x00000003, 0x00000000, 0xffffffff, 0xfffffffb,
        0xfffffffe, 0xffffffff, 0xfffffffd, 0x00000004,
    },
    0x00000001,
};

// from original: data for Montgomery operations modulo the curve's n
// may be, P256 curve (n) or related
static const m256_mod p256_n = {
    {
        0xFC632551, 0xF3B9CAC2, 0xA7179E84, 0xBCE6FAAD,
        0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
    },
    {
        0xbe79eea2, 0x83244c95, 0x49bd6fa6, 0x4699799c,
        0x2b6bec59, 0x2845b239, 0xf3d95620, 0x66e12d94,
    },
    0xee00bc4f,
};




// specific operation
static uint64_t u32_muladd64(uint32_t x, uint32_t y, uint32_t z, uint32_t t);

// set-ops
static void u256_set32(u256_t& z, uint32_t x);

//
static uint32_t u256_add(u256_t& z, u256_t const& x, u256_t const& y);
static uint32_t u256_sub(u256_t& z, u256_t const& x, u256_t const& y);

// mask ops
static void     u256_cmov(u256_t& z, u256_t const& x, uint32_t c);
static uint32_t u256_diff(u256_t const& x, u256_t const& y);
static uint32_t u256_diff(m256_t const& x);

// byself?
uint32_t u256_diff0(m256_t const& x);

// u288 math
static uint32_t u288_muladd  (uint32_t z[9], uint32_t x, const uint32_t y[8]);
static void     u288_rshift32(uint32_t z[9], uint32_t c);

//
static void u256_from_bytes(u256_t& a, byte_t const& b);
static void u256_to_bytes(byte_t& a, u256_t const& b);

//
static void m256_add  (u256_t& z, m256_t const& x, const m256_mod *mod);
static void m256_add_p(u256_t& z, m256_t const& x);
static void m256_sub  (u256_t& z, m256_t const& x, const m256_mod *mod);
static void m256_sub_p(u256_t& z, m256_t const& x);
static void m256_mul  (u256_t& z, m256_t const& x, const m256_mod *mod);
static void m256_mul_p(u256_t& z, m256_t const& x);
static void m256_inv  (u256_t& z, m256_t const& x, const m256_mod *mod);

//
static void m256_prep (u256_t& z, const m256_mod *mod);
static void m256_done (u256_t& z, const m256_mod *mod);
static void m256_set32(u256_t& z, uint32_t x, const m256_mod *mod);

//
static int  m256_from_bytes(u256_t& z, byte_t const& p, const m256_mod *mod);
static void m256_to_bytes (byte_t& p, u256_t& z       , const m256_mod *mod);

//
static uint32_t point_check(m256_t const& p);

//
static void point_to_affine(point_t& p, m256_t const& p); // make m256_z (or just u256_t& z argument) from m256_t
static void point_double(point_t const& p);
static void point_add(point_t &d, m256_t const& a);

//
static void point_add_or_double_leaky(m256_t& d, m256_t const& a, m256_t const& b);
static int  point_from_bytes(m256_t& p, const byte_2& p);
static void point_to_bytes(const byte_2& p, m256_t& p);

//
static void scalar_mult(m256_t &p, m256_t const& a, scalar_t const& b);
static int  scalar_from_bytes(scalar_t &s, byte_t const& a);

// generator
static int  scalar_gen_with_pub(byte_t& a, scalar_t &s, m256_t& p);
