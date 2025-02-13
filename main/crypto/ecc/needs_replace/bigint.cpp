#include "bigint.hpp"

//
#ifdef ENABLE_GMP_BIGINT
#include <stdexcept>
#include <iostream>

// Default Constructor
bigint_t::bigint_t() {
    mpz_init(value);
}

// Construct from unsigned long int
bigint_t::bigint_t(unsigned long int val) {
    mpz_init_set_ui(value, val);
}

// Construct from signed long int
bigint_t::bigint_t(signed long int val) {
    mpz_init_set_si(value, val);
}

// Construct from string
bigint_t::bigint_t(const std::string &val, int base) {
    if (mpz_init_set_str(value, val.c_str(), base) == -1) {
        throw std::invalid_argument("Invalid number string.");
    }
}

// Copy Constructor
bigint_t::bigint_t(const bigint_t &other) {
    mpz_init_set(value, other.value);
}

// Construct from mpz_t
bigint_t::bigint_t(mpz_t bi) {
    mpz_init_set(value, bi);
}

// Destructor
bigint_t::~bigint_t() {
    mpz_clear(value);
}

// Arithmetic Operations
bigint_t bigint_t::operator+(const bigint_t &other) const {
    bigint_t result;
    mpz_add(result.value, value, other.value);
    return result;
}

bigint_t bigint_t::operator-(const bigint_t &other) const {
    bigint_t result;
    mpz_sub(result.value, value, other.value);
    return result;
}

bigint_t bigint_t::operator*(const bigint_t &other) const {
    bigint_t result;
    mpz_mul(result.value, value, other.value);
    return result;
}

bigint_t bigint_t::operator/(const bigint_t &other) const {
    bigint_t result;
    mpz_fdiv_q(result.value, value, other.value);
    return result;
}

bigint_t bigint_t::operator%(const bigint_t &other) const {
    bigint_t result;
    mpz_mod(result.value, value, other.value);
    return result;
}

// Assignment Operations
bigint_t& bigint_t::operator=(const bigint_t &other) {
    mpz_set(value, other.value);
    return *this;
}

bigint_t& bigint_t::operator+=(const bigint_t &other) {
    mpz_add(value, value, other.value);
    return *this;
}

bigint_t& bigint_t::operator-=(const bigint_t &other) {
    mpz_sub(value, value, other.value);
    return *this;
}

bigint_t& bigint_t::operator*=(const bigint_t &other) {
    mpz_mul(value, value, other.value);
    return *this;
}

bigint_t& bigint_t::operator/=(const bigint_t &other) {
    mpz_fdiv_q(value, value, other.value);
    return *this;
}

bigint_t& bigint_t::operator%=(const bigint_t &other) {
    mpz_mod(value, value, other.value);
    return *this;
}

// Comparison Operations
bool bigint_t::operator==(const bigint_t &other) const {
    return mpz_cmp(value, other.value) == 0;
}

bool bigint_t::operator!=(const bigint_t &other) const {
    return !(*this == other);
}

bool bigint_t::operator<(const bigint_t &other) const {
    return mpz_cmp(value, other.value) < 0;
}

bool bigint_t::operator<=(const bigint_t &other) const {
    return mpz_cmp(value, other.value) <= 0;
}

bool bigint_t::operator>(const bigint_t &other) const {
    return mpz_cmp(value, other.value) > 0;
}

bool bigint_t::operator>=(const bigint_t &other) const {
    return mpz_cmp(value, other.value) >= 0;
}

// Utility Functions
std::string bigint_t::toString(int base) const {
    char* str = mpz_get_str(nullptr, base, value);
    std::string result(str);
    free(str);
    return result;
}

size_t bigint_t::bitSize() const {
    return mpz_sizeinbase(value, 2);
}

bool bigint_t::isZero() const {
    return mpz_cmp_ui(value, 0) == 0;
}

void bigint_t::print() const {
    mpz_out_str(stdout, 10, value);
    printf("\n");
}

void bigint_t::negate() {
    mpz_neg(value, value);
}

bool bigint_t::invert(const bigint_t& modulus) {
    return mpz_invert(value, value, modulus.value) != 0;
}

bigint_t bigint_t::modInverse(const bigint_t& modulus) const {
    bigint_t inverse;
    if (mpz_invert(inverse.value, value, modulus.value) != 0) {
        return inverse;
    } else {
        throw std::runtime_error("Modular inverse does not exist.");
    }
}


// Primality Testing
int bigint_t::isPrime(int provable) const {
    return mpz_probab_prime_p(value, provable);
}

// Next Prime
bigint_t bigint_t::nextPrime(const bigint_t& startAt, int provable) {
    bigint_t result;
    mpz_nextprime(result.value, startAt.value);
    return result;
}

// Bitwise Operations
bigint_t bigint_t::bitwiseAnd(const bigint_t& other) const {
    bigint_t result;
    mpz_and(result.value, value, other.value);
    return result;
}

bigint_t bigint_t::bitwiseIor(const bigint_t& other) const {
    bigint_t result;
    mpz_ior(result.value, value, other.value);
    return result;
}

bigint_t bigint_t::bitwiseXor(const bigint_t& other) const {
    bigint_t result;
    mpz_xor(result.value, value, other.value);
    return result;
}

bigint_t bigint_t::bitwiseComplement() const {
    bigint_t result;
    mpz_com(result.value, value);
    return result;
}

bigint_t bigint_t::leftShift(unsigned long int shiftBy) const {
    bigint_t result;
    mpz_mul_2exp(result.value, value, shiftBy);
    return result;
}

bigint_t bigint_t::rightShift(unsigned long int shiftBy) const {
    bigint_t result;
    mpz_fdiv_q_2exp(result.value, value, shiftBy);
    return result;
}

bool bigint_t::isNegative() const {
    return mpz_sgn(value) < 0;
}

void bigint_t::printAbsolute() const {
    mpz_t absValue;
    mpz_init(absValue);
    mpz_abs(absValue, value);
    mpz_out_str(stdout, 10, absValue);
    mpz_clear(absValue);
}

void bigint_t::addByte(unsigned char byte) {
    mpz_mul_2exp(value, value, 8);
    mpz_add_ui(value, value, byte);
}

bigint_t bigint_t::generateRandom(size_t numBits) {
    bigint_t randNum;
    size_t numbytes_t = (numBits + 7) / 8;

    #if defined(_WIN32) || defined(_WIN64)
    // std:: cout <<" Entering here \n";
    HCRYPTPROV hProvider = 0;
    BYTE* buffer = new BYTE[numbytes_t];
    if (CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProvider, numbytes_t, buffer);
        mpz_import(randNum.value, numbytes_t, 1, sizeof(buffer[0]), 0, 0, buffer);
        CryptReleaseContext(hProvider, 0);
    }
    delete[] buffer;

    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd != -1) {
        char* buffer = new char[numbytes_t];
        read(fd, buffer, numbytes_t);
        mpz_import(randNum.value, numbytes_t, 1, sizeof(buffer[0]), 0, 0, buffer);
        close(fd);
        delete[] buffer;
    }

    #else
    gmp_randstate_t state;
    gmp_randinit_default(state);
    mpz_urandomb(randNum.value, state, numBits);
    gmp_randclear(state);
    #endif
    // std:: cout <<" Working \n";
    // randNum.print();
    return randNum;
}


bigint_t stringTobigint_t(const std::string& str) {
    bigint_t result;
    for (unsigned char c : str) {
        result.addByte(c);
    }
    return result;
}

std::string bigIntToString(const bigint_t& bigint) {
    std::string result;
    bigint_t current = bigint;
    bigint_t zero(static_cast<unsigned long int>(0));
    const bigint_t byteSize(static_cast<unsigned long int>(256));

    while (current > zero) {
        try {
            unsigned long remainderValue = (current % byteSize).toULongSafe();
            unsigned char byte = static_cast<unsigned char>(remainderValue);
            result.insert(0, 1, byte);
            current /= byteSize;
        } catch (const std::overflow_error& e) {
            break;
        }
    }

    return result;
}
#endif
