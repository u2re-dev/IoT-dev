
//////////////////////////////////////////////////////////
// 1. Математические функции (модульное деление, powMod, sqrt и т.д.)
//////////////////////////////////////////////////////////
namespace MathUtil {
    // Функция для приведения к модулю (a mod m)
    //inline uint256_t mod(uint256_t a, uint256_t m) {
        //return a % m; // доработайте обработку отрицательных значений, если необходимо.
    //}

    // Возвращает положительный остаток от деления dividend на divisor.
    template<typename T>
    T mod(const T dividend, const T divisor) {
        T r = dividend % divisor;
        return (r >= 0) ? r : r + divisor;
    }

    // Функция возведения в степень по модулю (подход getPowMod из TS-кода).
    // floorFn – функция целочисленного деления; для встроенных целых типов стандартное деление уже округляет вниз.
    template<typename T, typename FloorFn>
    T powMod(const T base, T exponent, const T modulus, FloorFn floorFn) {
        if(modulus == T(1))
            return T(0);
        T result = T(1);
        T b = mod(base, modulus);
        while(exponent > T(0)) {
            if( (exponent % T(2)) == T(1) ) {
                result = mod(result * b, modulus);
            }
            exponent = floorFn(exponent / T(2));
            b = mod(b * b, modulus);
        }
        return result;
    }

    // Для целых типов C++ деление уже является целочисленным округлением вниз.
    template<typename T>
    T powModBI(const T base, T exponent, const T modulus) {
        if(modulus == T(1))
            return T(0);
        T result = T(1);
        T b = mod(base, modulus);
        while(exponent > T(0)) {
            if((exponent % T(2)) == T(1))
                result = mod(result * b, modulus);
            exponent /= T(2);
            b = mod(b * b, modulus);
        }
        return result;
    }

    // Простой вариант squareRoot для double
    inline double squareRoot(const double n) {
        return std::sqrt(n);
    }

    // Извлечение целочисленного квадратного корня для BigInt методом Ньютона.
    // Здесь выбран начальный приближённый корень по количеству цифр (очень упрощённо).
    inline BigInt squareRootBI(const BigInt n) {
        if(n == 0) return 0;
        if(n < 4) return 1;
        if(n < 9) return 2;
        if(n < 16) return 3;
        // Получаем количество десятичных цифр (путём преобразования в строку)
        std::ostringstream oss;
        oss << n;
        std::string s = oss.str();
        size_t half = s.size() / 2;
        BigInt res = std::stoull(s.substr(0, half)); //начальное приближение
        BigInt last = 0;
        while (true) {
            last = res;
            res = (res + n / res) / 2;
            BigInt p = res * res;
            if(p == n)
                return res;
            if(last == res)
                return res;
            // Дополнительная проверка (как в TS-коде)
            BigInt next = p + res * 2 - 1;
            if(n > next)
                return res;
        }
        return res;
    }

    // Алгоритм Тонелли–Шэнкса для поиска квадратного корня по модулю простого числа p (n < p).
    // Возвращает r, такое что r*r = n (mod p) или 0, если решения нет.
    inline BigInt tonelliShanksBI(BigInt n, BigInt p) {
        BigInt s = 0;
        BigInt q = p - 1;
        while ((q & 1) == 0) {
            q /= 2;
            s++;
        }
        if(s == 1) {
            BigInt r = powModBI(n, (p + 1) / 4, p);
            if (mod(r * r, p) == n)
                return r;
            return 0;
        }
        BigInt z = 1;
        // подбираем первый квадратично нерезидент z
        while(powModBI(++z, (p - 1) / 2, p) != (p - 1))
            ;
        BigInt c = powModBI(z, q, p);
        BigInt r = powModBI(n, (q + 1) / 2, p);
        BigInt t = powModBI(n, q, p);
        BigInt m = s;
        while(t != 1) {
            BigInt tt = t;
            BigInt i = 0;
            while(tt != 1) {
                tt = mod(tt * tt, p);
                i++;
                if(i == m)
                    return 0;
            }
            BigInt b = powModBI(c, powModBI(2, m - i - 1, p - 1), p);
            BigInt b2 = mod(b * b, p);
            r = mod(r * b, p);
            t = mod(t * b2, p);
            c = b2;
            m = i;
        }
        if(mod(r * r, p) == n)
            return r;
        return 0;
    }
} // namespace MathUtil
