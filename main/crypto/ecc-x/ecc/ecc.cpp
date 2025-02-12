#include "ecc.hpp"

//
#include <iostream>




//
static BigInt inv(BigInt num, BigInt mod) {
    if(num == 0 || mod <= 0) throw std::runtime_error("no inverse");
    BigInt a = mod(num, mod), b = mod;
    BigInt x = 0, y = 1, u = 1, v = 0;
    while(a != 0) {
        BigInt q = b / a;
        BigInt r = b % a;
        BigInt m = x - u * q;
        BigInt n = y - v * q;
        b = a; a = r; x = u; y = v; u = m; v = n;
    }
    if(b == 1) return mod(x, mod);
    throw std::runtime_error("no inverse");
}

//
Ecc_Point Ecc_Point::multiply(const BigInt &scalar) const {
    if(scalar == 0) return ZERO;
    if(scalar <= 0 || scalar >= N) throw std::runtime_error("scalar invalid");
    Ecc_Point result = ZERO;
    Ecc_Point addend = *this;
    BigInt n = scalar;
    while(n > 0) {
        if(n % 2 == 1)
            result = result.add(addend);
        addend = addend.doublePoint();
        n /= 2;
    }
    return result;
}

//
// Проверка равенства точек (по видимым аффинным координатам)
/*bool Ecc_Point::equals(const Ecc_Point &other) const {
    BigInt X1Z2 = mod(px * other.pz, P);
    BigInt X2Z1 = mod(other.px * pz, P);
    BigInt Y1Z2 = mod(py * other.pz, P);
    BigInt Y2Z1 = mod(other.py * pz, P);
    return (X1Z2 == X2Z1 && Y1Z2 == Y2Z1);
}*/

//
Ecc_Point Ecc_Point::doublePoint() const {
    if (this->isInfinity || yCoord.isZero()) {
        return Ecc_Point();
    }

    BigInt lambda = (BigInt(static_cast<unsigned long int>(3)) * xCoord * xCoord + curveParams.a) * inv(BigInt(static_cast<unsigned long int>(2)) * yCoord, curveParams.p);
    BigInt x3 = (lambda * lambda - BigInt(static_cast<unsigned long int>(2)) * xCoord) % curveParams.p;
    BigInt y3 = (lambda * (xCoord - x3) - yCoord) % curveParams.p;
    return Ecc_Point(x3, y3);
}

//
AffinePoint Ecc_Point::toAffine() const {
    if (equals(ZERO)) return {0, 0};
    if (pz == 1) return {px, py};
    BigInt iz = inv(pz, P);
    if (mod(pz * iz, P) != 1) throw std::runtime_error("inverse invalid");
    return { mod(px * iz, P), mod(py * iz, P) };
}

//
static Ecc_Point Ecc_Point::fromAffine(const AffinePoint &pt) {
    if(pt.x == 0 && pt.y == 0) return ZERO;
    return Ecc_Point(pt.x, pt.y, 1);
}

//
static Ecc_Point Ecc_Point::fromHex(const std::string &hexStr) {
    Bytes hex = HexUtil::hexToBytes(hexStr);
    size_t len = hex.size();
    if (len != 33 && len != 65) throw std::runtime_error("Invalid point hex length");

    // Извлекаем x – следующие 32 байта после префикса.
    Bytes xBytes(hex.begin() + 1, hex.begin() + 1 + 32);
    BigInt x = b2n(xBytes); // преобразование байтов в BigInt (функция b2n ниже)
    // Для сжатого представления определяем y через sqrt(x^3+7) с учетом четности.
    if (len == 33) {
        if (x <= 0 || x >= BASE.P) throw std::runtime_error("Point hex invalid: x not FE");
        BigInt lambda = mod(curve(x), BASE.P); // вычисляем x^3+7 mod P
        BigInt y = MathUtil::squareRootBI(lambda);
        bool isYOdd = (y & 1) == 1;
        bool headOdd = (hex[0] & 1) == 1; // если префикс 0x03 – нечётное, 0x02 – чётное
        if(isYOdd != headOdd)
            y = mod(-y, BASE.P);
        return Ecc_Point(x, y, 1).assertValidity();
    }
    // Если несжатый формат, x – первые 32 байта после префикса 0x04, y – следующие 32 байта.
    if (len == 65 && hex[0] == 0x04) {
        Bytes xB(hex.begin() + 1, hex.begin() + 1 + 32);
        Bytes yB(hex.begin() + 1 + 32, hex.end());
        BigInt xVal = b2n(xB);
        BigInt yVal = b2n(yB);
        return Ecc_Point(xVal, yVal, 1).assertValidity();
    }
    throw std::runtime_error("Point invalid: not on curve");
}





//
Ecc_Point Ecc_Point::assertValidity() const {
    AffinePoint a = toAffine();
    if(a.x <= 0 || a.x >= P || a.y <= 0 || a.y >= P) throw std::runtime_error("Point invalid: x or y");
    if (mod(a.y * a.y, P) != mod((a.x * a.x * a.x) + 7, P)) throw std::runtime_error("Point invalid: not on curve");
    return *this;
}

//
std::string Ecc_Point::toHex(bool isCompressed = true) const {
    AffinePoint a = toAffine();
    std::string head = isCompressed ? ((a.y & 1) == 0 ? "02" : "03") : "04";
    return head + n2h(a.x) + (isCompressed ? "" : n2h(a.y));
}

//─────────────────────────────────────────────────────────────
// Дополнительные преобразования для вывода числа в hex-строку (например, для отладки)
/*inline std::string n2h(const U256 &num) {
    std::ostringstream oss;
    for (size_t i = 0; i < num.size(); i++) { oss << std::hex << std::setw(8) << std::setfill('0') << num[num.size()-1-i]; };
    return oss.str();
}*/

//
static std::string Ecc_Point::n2h(const BigInt &num) {
    std::ostringstream oss;
    oss << std::hex << num;
    std::string s = oss.str();
    if(s.size() < 64) s = std::string(64 - s.size(), '0') + s;
    return s;
}

//
static BigInt b2n(const Bytes &b) {
    BigInt res = 0;
    for(auto byte : b)
        res = (res << 8) | byte;
    return res;
}




//
void Ecc_Point::print() const {
    if (isInfinity) {
        std::cout << "Point at Infinity" << std::endl;
    } else {
        std::cout << "Ecc_Point Coordinates:" << std::endl;
        std::cout << "x = " << xCoord.toString(16) << std::endl;
        std::cout << "y = " << yCoord.toString(16) << std::endl;
    }
}

//
Bytes Ecc_Point::toRawBytes(bool isCompressed = true) const {
    return HexUtil::hexToBytes(toHex(isCompressed));
}



//
static const CurveParameters& Ecc_Point::GetCurveParameters() {
    static CurveParameters staticCurveParams;
    static bool initialized = false;
    if (!initialized) {
        Ecc_Point temp;
        temp.setCurveParameters();
        temp.isInfinity=false;
        staticCurveParams = temp.curveParams;
        initialized = true;
    }
    return staticCurveParams;
}

//
void Ecc_Point::setCurveParameters() {
    #ifdef USE_CURVE_P256
        curveParams.a  = BigInt("ffffffff00000001000000000000000000000000fffffffffffffffffffffffc",16);
        curveParams.b  = BigInt("5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",16);
        curveParams.p  = BigInt("ffffffff00000001000000000000000000000000ffffffffffffffffffffffff",16);
        curveParams.Gx = BigInt("6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",16);
        curveParams.Gy = BigInt("4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",16);
        curveParams.n  = BigInt("ffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551",16);
    #elif defined USE_CURVE_SECP256K1
        curveParams.a  = BigInt("0",16);
        curveParams.b  = BigInt("7",16);
        curveParams.p  = BigInt("fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f",16);
        curveParams.Gx = BigInt("79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798",16);
        curveParams.Gy = BigInt("483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8",16);
        curveParams.n  = BigInt("fffffffffffffffffffffffe26f2fc170f69466a74defd8d",16);
    #elif defined USE_CURVE_P521
        curveParams.a  = BigInt("01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc",16);
        curveParams.b  = BigInt("0051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef109e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00",16);
        curveParams.p  = BigInt("01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",16);
        curveParams.Gx = BigInt("00c6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dbaa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",16);
        curveParams.Gy = BigInt("011839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650",16);
        curveParams.n  = BigInt("01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409",16);
    #elif defined USE_BLS12_381
        curveParams.a  = BigInt("0", 16);
        curveParams.b  = BigInt("4", 16);
        curveParams.p  = BigInt("1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab", 16);
        curveParams.Gx = BigInt("17f1d3a73197d7942695638c4fa9ac0fc3688c36f855e1bbf6c9b55bfd5a7a467e327a636e3bebf213b333b5d9c0d54a", 16);
        curveParams.Gy = BigInt("08b3f481e3aaa0f1a09e30ed741d8ae4fcf5e095d5d00af600db18cb2c04b3edd03cc744a2888ae40caa232946c5e7e1", 16);
        curveParams.n  = BigInt("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001", 16);
    #else
        #error "No elliptic curve defined"
    #endif
}
