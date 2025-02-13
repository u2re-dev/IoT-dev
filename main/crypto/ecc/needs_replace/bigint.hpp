#ifndef BIGINT_HPP
#define BIGINT_HPP

//
#ifdef ENABLE_GMP_BIGINT
#include <gmp.h>
#include <string>
#include <vector>
#include <limits>
#include <stdexcept>
#include <cstdint>

//
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <wincrypt.h>
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/stat.h>
#endif

/**
 * @class bigint_t
 * @brief A class for arbitrary-precision integers using the GMP library.
 */
class bigint_t {
public:
    /**
     * @brief Default constructor. Creates a bigint_t initialized to zero.
     */
    bigint_t();

    /**
     * @brief Constructor to initialize a bigint_t from an unsigned long int.
     * @param val The value to initialize the bigint_t with.
     */
    bigint_t(unsigned long int val);

    /**
     * @brief Constructor to initialize a bigint_t from a signed long int.
     * @param val The value to initialize the bigint_t with.
     */
    bigint_t(signed long int val);

    /**
     * @brief Constructor to initialize a bigint_t from a string representation.
     * @param val The string representation of the bigint_t.
     * @param base The base of the string representation (default is 10).
     */
    bigint_t(const std::string &val, int base = 10);

    /**
     * @brief Copy constructor.
     * @param other The bigint_t to copy.
     */
    bigint_t(const bigint_t &other);

    /**
     * @brief Constructor to initialize a bigint_t from a GMP mpz_t.
     * @param bi The GMP mpz_t to initialize from.
     */
    bigint_t(mpz_t bi);

    /**
     * @brief Destructor for bigint_t.
     */
    ~bigint_t();

    /**
     * @brief Addition operator.
     * @param other The bigint_t to add.
     * @return The result of the addition.
     */
    bigint_t operator+(const bigint_t &other) const;

    /**
     * @brief Subtraction operator.
     * @param other The bigint_t to subtract.
     * @return The result of the subtraction.
     */
    bigint_t operator-(const bigint_t &other) const;

    /**
     * @brief Multiplication operator.
     * @param other The bigint_t to multiply by.
     * @return The result of the multiplication.
     */
    bigint_t operator*(const bigint_t &other) const;

    /**
     * @brief Division operator.
     * @param other The bigint_t to divide by.
     * @return The result of the division.
     */
    bigint_t operator/(const bigint_t &other) const;

    /**
     * @brief Modulus operator.
     * @param other The bigint_t to take modulus with.
     * @return The result of the modulus operation.
     */
    bigint_t operator%(const bigint_t &other) const;

    /**
     * @brief Assignment operator.
     * @param other The bigint_t to assign.
     * @return Reference to this bigint_t after assignment.
     */
    bigint_t& operator=(const bigint_t &other);

    /**
     * @brief Addition assignment operator.
     * @param other The bigint_t to add.
     * @return Reference to this bigint_t after addition.
     */
    bigint_t& operator+=(const bigint_t &other);

    /**
     * @brief Subtraction assignment operator.
     * @param other The bigint_t to subtract.
     * @return Reference to this bigint_t after subtraction.
     */
    bigint_t& operator-=(const bigint_t &other);

    /**
     * @brief Multiplication assignment operator.
     * @param other The bigint_t to multiply by.
     * @return Reference to this bigint_t after multiplication.
     */
    bigint_t& operator*=(const bigint_t &other);

    /**
     * @brief Division assignment operator.
     * @param other The bigint_t to divide by.
     * @return Reference to this bigint_t after division.
     */
    bigint_t& operator/=(const bigint_t &other);

    /**
     * @brief Modulus assignment operator.
     * @param other The bigint_t to take modulus with.
     * @return Reference to this bigint_t after modulus operation.
     */
    bigint_t& operator%=(const bigint_t &other);

    /**
     * @brief Equality operator.
     * @param other The bigint_t to compare with.
     * @return True if equal, false otherwise.
     */
    bool operator==(const bigint_t &other) const;

    /**
     * @brief Inequality operator.
     * @param other The bigint_t to compare with.
     * @return True if not equal, false otherwise.
     */
    bool operator!=(const bigint_t &other) const;

    /**
     * @brief Less than operator.
     * @param other The bigint_t to compare with.
     * @return True if less than, false otherwise.
     */
    bool operator<(const bigint_t &other) const;

    /**
     * @brief Less than or equal to operator.
     * @param other The bigint_t to compare with.
     * @return True if less than or equal to, false otherwise.
     */
    bool operator<=(const bigint_t &other) const;

    /**
     * @brief Greater than operator.
     * @param other The bigint_t to compare with.
     * @return True if greater than, false otherwise.
     */
    bool operator>(const bigint_t &other) const;

    /**
     * @brief Greater than or equal to operator.
     * @param other The bigint_t to compare with.
     * @return True if greater than or equal to, false otherwise.
     */
    bool operator>=(const bigint_t &other) const;

    /**
     * @brief Convert bigint_t to a string representation.
     * @param base The base of the string representation (default is 10).
     * @return String representation of bigint_t.
     */
    std::string toString(int base = 10) const;

    /**
     * @brief Get the number of bits in the bigint_t.
     * @return Number of bits.
     */
    size_t bitSize() const;

    /**
     * @brief Check if bigint_t is zero.
     * @return True if zero, false otherwise.
     */
    bool isZero() const;

    /**
     * @brief Print the bigint_t to standard output.
     */
    void print() const;

    /**
     * @brief Check if bigint_t is negative.
     * @return True if negative, false otherwise.
     */
    bool isNegative() const;

    /**
     * @brief Negate the bigint_t.
     */
    void negate();

    /**
     * @brief Invert the bigint_t modulo a given modulus.
     * @param modulus The modulus to invert with.
     * @return True if inversion is successful, false otherwise.
     */
    bool invert(const bigint_t& modulus);

    /**
     * @brief Calculate the modular inverse of the bigint_t.
     * @param modulus The modulus for the modular inverse.
     * @return The modular inverse of the bigint_t.
     */
    bigint_t modInverse(const bigint_t& modulus) const;

    /**
     * @brief Print the absolute value of the bigint_t.
     */
    void printAbsolute() const;

    /**
     * @brief Check if the bigint_t is a prime number.
     * @param provable Flag to indicate provable primality testing.
     * @return 2 if definitely prime, 1 if probably prime, 0 if not prime.
     */
    int isPrime(int provable) const;

    /**
     * @brief Find the next prime number greater than or equal to the given bigint_t.
     * @param startAt The bigint_t to start searching for the next prime.
     * @param provable Flag to indicate provable primality testing.
     * @return The next prime number.
     */
    static bigint_t nextPrime(const bigint_t& startAt, int provable);

    /**
     * @brief Bitwise AND operation with another bigint_t.
     * @param other The bigint_t to perform bitwise AND with.
     * @return The result of the bitwise AND operation.
     */
    bigint_t bitwiseAnd(const bigint_t& other) const;

    /**
     * @brief Bitwise OR operation with another bigint_t.
     * @param other The bigint_t to perform bitwise OR with.
     * @return The result of the bitwise OR operation.
     */
    bigint_t bitwiseIor(const bigint_t& other) const;

    /**
     * @brief Bitwise XOR operation with another bigint_t.
     * @param other The bigint_t to perform bitwise XOR with.
     * @return The result of the bitwise XOR operation.
     */
    bigint_t bitwiseXor(const bigint_t& other) const;

    /**
     * @brief Bitwise complement (NOT) operation.
     * @return The result of the bitwise complement operation.
     */
    bigint_t bitwiseComplement() const;

    /**
     * @brief Left shift operation by a specified number of bits.
     * @param shiftBy The number of bits to shift left by.
     * @return The result of the left shift operation.
     */
    bigint_t leftShift(unsigned long int shiftBy) const;

    /**
     * @brief Right shift operation by a specified number of bits.
     * @param shiftBy The number of bits to shift right by.
     * @return The result of the right shift operation.
     */
    bigint_t rightShift(unsigned long int shiftBy) const;

    /**
     * @brief Generates a random bigint_t using the platform-specific PRNG or GMP's PRNG as a fallback.
     * @param numBits The number of bits in the desired random bigint_t.
     * @return A random bigint_t of the specified bit size.
     */
    static bigint_t generateRandom(size_t numBits);

    /**
     * @brief Adds a single byte to the bigint_t value.
     *
     * This method shifts the current bigint_t value left by 8 bits (1 byte) to make room for the new byte and then adds the byte to the bigint_t. This is equivalent to multiplying the bigint_t by 256 and adding the byte value. It is useful for constructing a bigint_t from a binary representation, such as converting a string to a bigint_t where each character is treated as a byte.
     *
     * @param byte The byte to add to the bigint_t. It is treated as an unsigned value, so negative numbers will be converted to their unsigned equivalent.
     */
    void addByte(unsigned char byte);

    /**
     * @brief Converts the bigint_t to an unsigned long safely.
     *
     * Attempts to convert the bigint_t to an unsigned long. If the bigint_t value
     * is too large to fit in an unsigned long, it either returns the maximum
     * unsigned long value or throws an overflow exception.
     *
     * @return The bigint_t value as an unsigned long, or the maximum value for
     * unsigned long if the bigint_t is too large.
     * @throws std::overflow_error if the bigint_t value exceeds the range of
     * an unsigned long.
     */
    unsigned long toULongSafe() const {
        if (mpz_fits_ulong_p(value) != 0) {
            return mpz_get_ui(value);
        } else {
            throw std::overflow_error("bigint_t value exceeds the range of unsigned long");
        }
    }

    /**
     * @brief Converts a bigint_t to its string representation.
     *
     * This function takes a bigint_t object and converts it to a string. The conversion process involves extracting each byte from the bigint_t, starting from the least significant byte, and constructing the corresponding string. This is particularly useful for converting large numerical values, stored in a bigint_t, back to a human-readable string format.
     *
     * @param bigint The bigint_t object to be converted to a string.
     * @return A string representation of the bigint_t object.
     */
    std::string bigIntToString(const bigint_t& bigint);

    /**
     * @brief Converts a string to a bigint_t object.
     *
     * This function takes a string representation of a number and converts it into a bigint_t object. It is capable of handling very large numbers that are represented as strings, converting them into a bigint_t format for further numerical manipulations. Each character in the string is treated as a byte in the base-256 number system, allowing for the conversion of arbitrary binary data into a bigint_t.
     *
     * @param str The string to be converted to a bigint_t object. The string can represent a very large number or arbitrary binary data.
     * @return A bigint_t object representing the numerical value of the input string.
     */
    bigint_t stringTobigint_t(const std::string& str);


private:
    mpz_t value; // The GMP mpz_t representing the bigint_t.
};

#endif // BIGINT_HPP
#endif
