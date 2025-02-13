#pragma once
#include "./types.hpp"

//
namespace bmath {
        // Функция возведения в степень по модулю (подход getPowMod из TS-кода).
    // floorFn – функция целочисленного деления; для встроенных целых типов стандартное деление уже округляет вниз.
    template<typename FloorFn, typename T = bigint_t>
    inline T powMod(const T base, T exponent, const T modulus, FloorFn floorFn) {
        if (modulus == T(1)) return T(0);
        T result = T(1);
        T b = mod(base, modulus);
        while (exponent > T(0)) {
            if ((exponent % T(2)) == T(1)) 
                { result = mod(result * b, modulus); }
            exponent = floorFn(exponent / T(2));
            b = mod(b * b, modulus);
        }
        return result;
    }

    // Для целых типов C++ деление уже является целочисленным округлением вниз.
    template<typename T = bigint_t>
    inline T powModBI(const T base, T exponent, const T modulus) {
        if (modulus == T(1)) return T(0);
        T result = T(1);
        T b = mod(base, modulus);
        while (exponent > T(0)) {
            if ((exponent % T(2)) == T(1)) result = mod(result * b, modulus);
            exponent /= T(2); b = mod(b * b, modulus);
        }
        return result;
    }

    //
    template<typename T = bigint_t>
    inline T mod(const T dividend, const T divisor) {
        T r = dividend % divisor;
        return (r >= 0) ? r : r + divisor;
    }

    //
    inline void zeroize(void* d, size_t n) {
        volatile uint8_t* p = reinterpret_cast<volatile uint8_t*>(d);
        while (n--) *p++ = 0;
    }

    //
    inline bigint_t mod(const bigint_t dividend, const bigint_t divisor) {
        bigint_t r = dividend % divisor;
        return (r >= 0) ? r : r + divisor;
    }

    //
    inline bigint_t curve(bigint_t x, bigint_t B, bigint_t P) {
        return mod(mod(x * x, P) * x + B);
    };

    // Простой вариант squareRoot для double
    inline double squareRoot(const double n) {
        return std::sqrt(n);
    }

    //
    inline bigint_t inv(bigint_t num, bigint_t mod) {
        if (num == 0 || mod <= 0) throw std::runtime_error("no inverse");
        bigint_t a = mod(num, mod), b = mod;
        bigint_t x = 0, y = 1, u = 1, v = 0;
        while (a != 0) {
            bigint_t q = b / a;
            bigint_t r = b % a;
            bigint_t m = x - u * q;
            bigint_t n = y - v * q;
            b = a; a = r; x = u; y = v; u = m; v = n;
        }
        if (b == 1) return mod(x, mod);
        throw std::runtime_error("no inverse");
    }

    //
    inline bigint_t squareRootBI(const bigint_t n) {
        if (n == 0) return 0;
        if (n < 4)  return 1;
        if (n < 9)  return 2;
        if (n < 16) return 3;

        //
        std::ostringstream oss; oss << n;
        std::string s = oss.str();
        size_t half = s.size() / 2;
        bigint_t res = std::stoull(s.substr(0, half));
        bigint_t last = 0;
        while (true) {
            last = res; res = (res + n / res) / 2;
            bigint_t p = res * res;
            if (p == n) return res;
            if (last == res) return res;

            //
            bigint_t next = p + res * 2 - 1;
            if (n > next) return res;
        }
        return res;
    }

    // Алгоритм Тонелли–Шэнкса для поиска квадратного корня по модулю простого числа p (n < p).
    // Возвращает r, такое что r*r = n (mod p) или 0, если решения нет.
    inline bigint_t tonelliShanksBI(bigint_t n, bigint_t p) {
        bigint_t s = 0;
        bigint_t q = p - 1;
        while ((q & 1) == 0) { q /= 2; s++; }
        if (s == 1) {
            bigint_t r = powModBI(n, (p + 1) / 4, p);
            if (mod(r * r, p) == n) return r;
            return 0;
        }
        bigint_t z = 1;

        // подбираем первый квадратично нерезидент z
        while (powModBI(++z, (p - 1) / 2, p) != (p - 1));

        //
        bigint_t c = powModBI(z, q, p);
        bigint_t r = powModBI(n, (q + 1) / 2, p);
        bigint_t t = powModBI(n, q, p);
        bigint_t m = s;
        while (t != 1) {
            bigint_t tt = t;
            bigint_t i = 0;
            while (tt != 1) {
                tt = mod(tt * tt, p); i++;
                if (i == m) return 0;
            }
            bigint_t b = powModBI(c, powModBI(2, m - i - 1, p - 1), p);
            bigint_t b2 = mod(b * b, p);
            r = mod(r * b, p);
            t = mod(t * b2, p);
            c = b2;
            m = i;
        }
        if (mod(r * r, p) == n) return r;
        return 0;
    }
}
