// =====================================================================
// Параметры для работы PBKDF2
struct PbkdfParameters {
    uint32_t iterations;
    ByteArray salt;
};

// =====================================================================
// Криптографические утилиты (здесь представлены только интерфейсы)
// Эти функции можно заменить/сконфигурировать под конкретную платформу (например, ESP32-S3)
namespace Crypto {
    // PBKDF2; если отсутствует реализация, можно взять свою или считать доступной
    ByteArray pbkdf2(const ByteArray& input, const ByteArray& salt, uint32_t iterations, size_t outputLength);

    // HKDF: key derivation
    ByteArray hkdf(const ByteArray& ikm, const ByteArray& salt, const ByteArray& info, size_t L);

    // HMAC (например, HMAC-SHA256)
    ByteArray hmac(const ByteArray& key, const ByteArray& data);

    // Хеш-функция (например, SHA256)
    ByteArray hash(const ByteArray& data);

    // Генерация случайного скалярного значения в диапазоне [1, order-1]
    uint256_t getRandomBigInt(size_t numBytes, const uint256_t& order);
}
