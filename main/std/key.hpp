// Приведение приватного ключа к числовому типу.
// Если ключ передан в виде hex‑строки, преобразуем его в BigInt; иначе – предполагаем, что уже BigInt.
inline BigInt toPriv(const std::string &priv) {
    // Здесь простая реализация: конвертируем из hex в BigInt.
    BigInt d = std::stoull(priv, nullptr, 16);
    if(d > 0 && d < ECCPoint::N)
        return d;
    throw std::runtime_error("private key invalid");
}

// Получение публичного ключа из приватного ключа (вычисление Q = G * d).
inline Bytes getPublicKey(const std::string &priv, bool isCompressed = true) {
    BigInt d = toPriv(priv);
    ECCPoint Q = ECCPoint::BASE.multiply(d);
    return Q.toRawBytes(isCompressed);
}
