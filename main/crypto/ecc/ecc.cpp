#include "ecc.hpp"

//
#include <iostream>

//
eccp_t ZERO = eccp_t(0, 1, 0);

//?======================================================
//? Math specific implementation (except operators)

//
eccp_t eccp_t::multiply(const bigint_t &scalar) const {
    if (scalar == 0 || this->isInfinity) { return ZERO; }
    eccp_t result = ZERO;
    eccp_t point  = *this;

    bigint_t k = scalar;
    while (k > bigint_t(static_cast<unsigned long int>(0))) {
        if (k % bigint_t(static_cast<unsigned long int>(2)) != bigint_t(static_cast<unsigned long int>(0))) {
            result = result + point;
        }
        point = point.doublePoint();
        k /= bigint_t(static_cast<unsigned long int>(2));
    }
    return result;
}

//
eccp_t eccp_t::doublePoint() const {
    //if (this->isInfinity || yCoord.isZero()) { return eccp_t(); };
    if (this->isInfinity || yCoord == 0) { return eccp_t(); };
    bigint_t lambda = (bigint_t(static_cast<unsigned long int>(3)) * xCoord * xCoord + curveParams.a) * bmath::inv(bigint_t(static_cast<unsigned long int>(2)) * yCoord, curveParams.p);
    bigint_t x3 = (lambda * lambda - bigint_t(static_cast<unsigned long int>(2)) * xCoord) % curveParams.p;
    bigint_t y3 = (lambda * (xCoord - x3) - yCoord) % curveParams.p;
    return eccp_t(x3, y3);
}

//
affine_t eccp_t::toAffine() const {
    if (*this == ZERO) return {0, 0};
    if (zCoord == 1) return {xCoord, yCoord};
    auto& P = curveParams.p;
    bigint_t iz = bmath::inv(zCoord, P);
    if (bmath::mod(zCoord * iz, P) != 1) throw std::runtime_error("inverse invalid");
    return { 
        bmath::mod(xCoord * iz, P), 
        bmath::mod(yCoord * iz, P) 
    };
}




//
eccp_t eccp_t::assertValidity() const {
    affine_t a = toAffine();
    auto P = curveParams.p;
    if(a.x <= 0 || a.x >= P || a.y <= 0 || a.y >= P) throw std::runtime_error("Point invalid: x or y");
    if (bmath::mod(a.y * a.y, P) != bmath::mod((a.x * a.x * a.x) + 7, P)) throw std::runtime_error("Point invalid: not on curve");
    return *this;
}

// deprecated
//bytes_t eccp_t::toRawbytes(bool isCompressed = true) const {
    //return hex::hexToBytes(toHex(isCompressed));
//}

//
bytes_t eccp_t::toBytes(bool isCompressed) const {
    bytes_t b = bytes_t(65);
    toBytes(b.data(), isCompressed);
    return b;
}

//
void eccp_t::toBytes(uint8_t* output, bool isCompressed) const {
    affine_t a = toAffine();
    if (isCompressed) {
        output[0] = (a.y & 1) == 0 ? 0x02 : 0x03;
        hex::n2b(a.x, output + 1, 32);
    } else {
        output[0] = 0x04;
        hex::n2b(a.x, output + 1, 32);
        hex::n2b(a.y, output + 17, 32);
    }
}

//
std::string eccp_t::toHex(bool isCompressed) const {
    affine_t a = toAffine();
    std::string head = isCompressed ? ((a.y & 1) == 0 ? "02" : "03") : "04";
    return head + hex::n2h(a.x) + (isCompressed ? "" : hex::n2h(a.y));
}

//
void eccp_t::print() const {
    if (isInfinity) {
        std::cout << "Point at Infinity" << std::endl;
    } else {
        std::cout << "eccp_t Coordinates:" << std::endl;
        std::cout << "x = " << xCoord.str(16) << std::endl;
        std::cout << "y = " << yCoord.str(16) << std::endl;
    }
}




//
void eccp_t::setCurveParameters() {
    #ifdef USE_CURVE_P256
        curveParams.a  = bigint_t("ffffffff00000001000000000000000000000000fffffffffffffffffffffffc",16);
        curveParams.b  = bigint_t("5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",16);
        curveParams.p  = bigint_t("ffffffff00000001000000000000000000000000ffffffffffffffffffffffff",16);
        curveParams.Gx = bigint_t("6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",16);
        curveParams.Gy = bigint_t("4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",16);
        curveParams.n  = bigint_t("ffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551",16);
    #elif defined USE_CURVE_SECP256K1
        curveParams.a  = bigint_t("0",16);
        curveParams.b  = bigint_t("7",16);
        curveParams.p  = bigint_t("fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f",16);
        curveParams.Gx = bigint_t("79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798",16);
        curveParams.Gy = bigint_t("483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8",16);
        curveParams.n  = bigint_t("fffffffffffffffffffffffe26f2fc170f69466a74defd8d",16);
    #elif defined USE_CURVE_P521
        curveParams.a  = bigint_t("01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc",16);
        curveParams.b  = bigint_t("0051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef109e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00",16);
        curveParams.p  = bigint_t("01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",16);
        curveParams.Gx = bigint_t("00c6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dbaa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",16);
        curveParams.Gy = bigint_t("011839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650",16);
        curveParams.n  = bigint_t("01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409",16);
    #elif defined USE_BLS12_381
        curveParams.a  = bigint_t("0", 16);
        curveParams.b  = bigint_t("4", 16);
        curveParams.p  = bigint_t("1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab", 16);
        curveParams.Gx = bigint_t("17f1d3a73197d7942695638c4fa9ac0fc3688c36f855e1bbf6c9b55bfd5a7a467e327a636e3bebf213b333b5d9c0d54a", 16);
        curveParams.Gy = bigint_t("08b3f481e3aaa0f1a09e30ed741d8ae4fcf5e095d5d00af600db18cb2c04b3edd03cc744a2888ae40caa232946c5e7e1", 16);
        curveParams.n  = bigint_t("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001", 16);
    #else
        #error "No elliptic curve defined"
    #endif
}



//?======================================================
//? Unused functions

//
/*inline std::string n2h(const U256 &num) {
    std::ostringstream oss;
    for (size_t i = 0; i < num.size(); i++) { oss << std::hex << std::setw(8) << std::setfill('0') << num[num.size()-1-i]; };
    return oss.str();
}*/

//
// Проверка равенства точек (по видимым аффинным координатам)
/*bool eccp_t::equals(const eccp_t &other) const {
    bigint_t X1Z2 = bmath::mod(px * other.pz, P);
    bigint_t X2Z1 = bmath::mod(other.px * pz, P);
    bigint_t Y1Z2 = bmath::mod(py * other.pz, P);
    bigint_t Y2Z1 = bmath::mod(other.py * pz, P);
    return (X1Z2 == X2Z1 && Y1Z2 == Y2Z1);
}*/
