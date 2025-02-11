#include "bigint.hpp"
#include <stdexcept>
#include <iostream>

// Default Constructor
BigInt::BigInt() {
    mpz_init(value);
}

// Construct from unsigned long int
BigInt::BigInt(unsigned long int val) {
    mpz_init_set_ui(value, val);
}

// Construct from signed long int
BigInt::BigInt(signed long int val) {
    mpz_init_set_si(value, val);
}

// Construct from string
BigInt::BigInt(const std::string &val, int base) {
    if (mpz_init_set_str(value, val.c_str(), base) == -1) {
        throw std::invalid_argument("Invalid number string.");
    }
}

// Copy Constructor
BigInt::BigInt(const BigInt &other) {
    mpz_init_set(value, other.value);
}

// Construct from mpz_t
BigInt::BigInt(mpz_t bi) {
    mpz_init_set(value, bi);
}

// Destructor
BigInt::~BigInt() {
    mpz_clear(value);
}

// Arithmetic Operations
BigInt BigInt::operator+(const BigInt &other) const {
    BigInt result;
    mpz_add(result.value, value, other.value);
    return result;
}

BigInt BigInt::operator-(const BigInt &other) const {
    BigInt result;
    mpz_sub(result.value, value, other.value);
    return result;
}

BigInt BigInt::operator*(const BigInt &other) const {
    BigInt result;
    mpz_mul(result.value, value, other.value);
    return result;
}

BigInt BigInt::operator/(const BigInt &other) const {
    BigInt result;
    mpz_fdiv_q(result.value, value, other.value);
    return result;
}

BigInt BigInt::operator%(const BigInt &other) const {
    BigInt result;
    mpz_mod(result.value, value, other.value);
    return result;
}

// Assignment Operations
BigInt& BigInt::operator=(const BigInt &other) {
    mpz_set(value, other.value);
    return *this;
}

BigInt& BigInt::operator+=(const BigInt &other) {
    mpz_add(value, value, other.value);
    return *this;
}

BigInt& BigInt::operator-=(const BigInt &other) {
    mpz_sub(value, value, other.value);
    return *this;
}

BigInt& BigInt::operator*=(const BigInt &other) {
    mpz_mul(value, value, other.value);
    return *this;
}

BigInt& BigInt::operator/=(const BigInt &other) {
    mpz_fdiv_q(value, value, other.value);
    return *this;
}

BigInt& BigInt::operator%=(const BigInt &other) {
    mpz_mod(value, value, other.value);
    return *this;
}

// Comparison Operations
bool BigInt::operator==(const BigInt &other) const {
    return mpz_cmp(value, other.value) == 0;
}

bool BigInt::operator!=(const BigInt &other) const {
    return !(*this == other);
}

bool BigInt::operator<(const BigInt &other) const {
    return mpz_cmp(value, other.value) < 0;
}

bool BigInt::operator<=(const BigInt &other) const {
    return mpz_cmp(value, other.value) <= 0;
}

bool BigInt::operator>(const BigInt &other) const {
    return mpz_cmp(value, other.value) > 0;
}

bool BigInt::operator>=(const BigInt &other) const {
    return mpz_cmp(value, other.value) >= 0;
}

// Utility Functions
std::string BigInt::toString(int base) const {
    char* str = mpz_get_str(nullptr, base, value);
    std::string result(str);
    free(str);
    return result;
}

size_t BigInt::bitSize() const {
    return mpz_sizeinbase(value, 2);
}

bool BigInt::isZero() const {
    return mpz_cmp_ui(value, 0) == 0;
}

void BigInt::print() const {
    mpz_out_str(stdout, 10, value);
    printf("\n");
}

void BigInt::negate() {
    mpz_neg(value, value);
}

bool BigInt::invert(const BigInt& modulus) {
    return mpz_invert(value, value, modulus.value) != 0;
}

BigInt BigInt::modInverse(const BigInt& modulus) const {
    BigInt inverse;
    if (mpz_invert(inverse.value, value, modulus.value) != 0) {
        return inverse;
    } else {
        throw std::runtime_error("Modular inverse does not exist.");
    }
}


// Primality Testing
int BigInt::isPrime(int provable) const {
    return mpz_probab_prime_p(value, provable);
}

// Next Prime
BigInt BigInt::nextPrime(const BigInt& startAt, int provable) {
    BigInt result;
    mpz_nextprime(result.value, startAt.value);
    return result;
}

// Bitwise Operations
BigInt BigInt::bitwiseAnd(const BigInt& other) const {
    BigInt result;
    mpz_and(result.value, value, other.value);
    return result;
}

BigInt BigInt::bitwiseIor(const BigInt& other) const {
    BigInt result;
    mpz_ior(result.value, value, other.value);
    return result;
}

BigInt BigInt::bitwiseXor(const BigInt& other) const {
    BigInt result;
    mpz_xor(result.value, value, other.value);
    return result;
}

BigInt BigInt::bitwiseComplement() const {
    BigInt result;
    mpz_com(result.value, value);
    return result;
}

BigInt BigInt::leftShift(unsigned long int shiftBy) const {
    BigInt result;
    mpz_mul_2exp(result.value, value, shiftBy);
    return result;
}

BigInt BigInt::rightShift(unsigned long int shiftBy) const {
    BigInt result;
    mpz_fdiv_q_2exp(result.value, value, shiftBy);
    return result;
}

bool BigInt::isNegative() const {
    return mpz_sgn(value) < 0;
}

void BigInt::printAbsolute() const {
    mpz_t absValue;
    mpz_init(absValue);
    mpz_abs(absValue, value);
    mpz_out_str(stdout, 10, absValue);
    mpz_clear(absValue);
}

void BigInt::addByte(unsigned char byte) {
    mpz_mul_2exp(value, value, 8);  
    mpz_add_ui(value, value, byte);
}

BigInt BigInt::generateRandom(size_t numBits) {
    BigInt randNum;
    size_t numBytes = (numBits + 7) / 8;

    #if defined(_WIN32) || defined(_WIN64)
    // std:: cout <<" Entering here \n";
    HCRYPTPROV hProvider = 0;
    BYTE* buffer = new BYTE[numBytes];
    if (CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProvider, numBytes, buffer);
        mpz_import(randNum.value, numBytes, 1, sizeof(buffer[0]), 0, 0, buffer);
        CryptReleaseContext(hProvider, 0);
    }
    delete[] buffer;

    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd != -1) {
        char* buffer = new char[numBytes];
        read(fd, buffer, numBytes);
        mpz_import(randNum.value, numBytes, 1, sizeof(buffer[0]), 0, 0, buffer);
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


BigInt stringToBigInt(const std::string& str) {
    BigInt result;
    for (unsigned char c : str) {
        result.addByte(c);
    }
    return result;
}

std::string bigIntToString(const BigInt& bigint) {
    std::string result;
    BigInt current = bigint;
    BigInt zero(static_cast<unsigned long int>(0));
    const BigInt byteSize(static_cast<unsigned long int>(256));

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
