#include "bigint.hpp"
#include "ecc.hpp"
#include <iostream>

//
Ecc_Point Ecc_Point::operator+(const Ecc_Point& other) const {
    if (this->isInfinity) return other;
    if (other.isInfinity) return *this;
    if (*this == -other) return Ecc_Point();
    if (*this == other) { return this->doublePoint(); }

    //
    BigInt lambda = (other.yCoord - yCoord) * ((other.xCoord - xCoord).modInverse(curveParams.p));
    BigInt x3 = (lambda * lambda - xCoord - other.xCoord) % curveParams.p;
    BigInt y3 = (lambda * (xCoord - x3) - yCoord) % curveParams.p;
    return Ecc_Point(x3, y3);
}

//
Ecc_Point Ecc_Point::operator-() const {
    if (this->isInfinity) return *this;
    return Ecc_Point(xCoord, curveParams.p - yCoord);
}

//
Ecc_Point Ecc_Point::operator*(const BigInt& scalar) const {
if (scalar.isZero() || this->isInfinity) {
    return Ecc_Point();
}
    Ecc_Point result;
    Ecc_Point point = *this;

    BigInt k = scalar;
    while (k > BigInt(static_cast<unsigned long int>(0))) {
        if (k % BigInt(static_cast<unsigned long int>(2)) != BigInt(static_cast<unsigned long int>(0))) {
            result = result + point;
        }
        point = point.doublePoint();
        k /= BigInt(static_cast<unsigned long int>(2));
    }
    return result;
}

bool Ecc_Point::operator==(const Ecc_Point& other) const {
    if (isInfinity && other.isInfinity) return true;
    if (isInfinity || other.isInfinity) return false;
    return (xCoord == other.xCoord) && (yCoord == other.yCoord);
}

Ecc_Point Ecc_Point::doublePoint() const {
    if (this->isInfinity || yCoord.isZero()) {
        return Ecc_Point();
    }

    BigInt lambda = (BigInt(static_cast<unsigned long int>(3)) * xCoord * xCoord + curveParams.a) *
                    (BigInt(static_cast<unsigned long int>(2)) * yCoord).modInverse(curveParams.p);
    BigInt x3 = (lambda * lambda - BigInt(static_cast<unsigned long int>(2)) * xCoord) % curveParams.p;
    BigInt y3 = (lambda * (xCoord - x3) - yCoord) % curveParams.p;
    return Ecc_Point(x3, y3);
}
