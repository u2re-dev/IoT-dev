#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <concepts>

// Допустим, наши типы BigInt выглядят так (реальные реализации могут быть значительно сложнее):
using uint128_t = __uint128_t;  // если поддерживается
using uint256_t = /* ваша реализация/обёртка над BigInt */ uint64_t; // заглушка – заменить на реальный BigInt

// Для удобства определяем тип для байтовых массивов:
using ByteArray = std::vector<uint8_t>;

// =====================================================================
// Предполагаем, что есть библиотека для работы с эллиптическими кривыми.
// Интерфейс класса Point (используем проективные координаты)
class Point {
public:
    // Координаты проективной точки
    // (в реальной реализации это могут быть uint256_t или подобные типы)
    uint256_t px, py, pz;

    // Конструктор
    Point(uint256_t x, uint256_t y, uint256_t z) : px(x), py(y), pz(z) {}

    // Возвращает базовую точку (генератор)
    static Point Base();

    // Возвращает точку смещения (нулевую точку/точку идентичности)
    static Point Zero();

    // Создать точку из аффинных координат (x,y)
    static Point fromAffine(uint256_t x, uint256_t y);

    // Парсинг точки из массива байт (например, в формате uncompressed)
    static Point fromBytes(const ByteArray& bytes);

    // Возвращает аффинное представление точки
    std::pair<uint256_t, uint256_t> toAffine() const;

    // Проверка корректности точки – выбрасывает исключение, если точка не на кривой
    void assertValidity() const;

    // Операции сложения
    Point add(const Point& other) const;

    // Умноже­ние точки на скаляр (предполагается, что n > 0 и находится в диапазоне)
    Point multiply(const uint256_t& scalar) const;

    // Операция вычитания
    Point subtract(const Point& other) const {
        // Реализуем через сложение с отрицанием
        return add(other.negate());
    }

    // Возвращает точку с инвертированной y-координатой
    Point negate() const;

    // Код для сериализации в байты (uncompressed формат)
    ByteArray toBytes(bool compressed = true) const;

    // Другие необходимые методы ...
};

// Здесь должны быть реализации Point::Base(), toAffine() и т.п., либо их прототипы,
// если они уже реализованы в отдельном модуле.

// =====================================================================
// Криптографические утилиты (здесь представлены только интерфейсы)
// Эти функции можно заменить/сконфигурировать под конкретную платформу (например, ESP32-S3)
namespace Crypto {
    // PBKDF2; если отсутствует реализация, можно взять свою или считать доступной
    ByteArray pbkdf2(const ByteArray& input, const ByteArray& salt,
                      uint32_t iterations, size_t outputLength);

    // HKDF: key derivation
    ByteArray hkdf(const ByteArray& ikm, const ByteArray& salt,
                   const ByteArray& info, size_t L);

    // HMAC (например, HMAC-SHA256)
    ByteArray hmac(const ByteArray& key, const ByteArray& data);

    // Хеш-функция (например, SHA256)
    ByteArray hash(const ByteArray& data);

    // Генерация случайного скалярного значения в диапазоне [1, order-1]
    uint256_t getRandomBigInt(size_t numBytes, const uint256_t& order);
}

// =====================================================================
// DataWriter – простой класс для записи данных в поток с префиксом длины в little-endian
class DataWriter {
public:
    ByteArray buffer;

    // Записываем 64-битное число (могут быть uint32_t, uint64_t и т.п.)
    void writeUInt64(uint64_t value) {
        for (unsigned i = 0; i < 8; i++) {
            buffer.push_back(static_cast<uint8_t>(value >> (8 * i)));
        }
    }

    // Запись массива байт: сначала длина, затем сами данные
    void writeByteArray(const ByteArray& data) {
        writeUInt64(data.size());
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    // Получить полностью сформированный массив
    ByteArray toByteArray() const { return buffer; }
};

// =====================================================================
// Параметры для работы PBKDF2
struct PbkdfParameters {
    uint32_t iterations;
    ByteArray salt;
};

// =====================================================================
// Структура, которая будет возвращаться в computeSecretAndVerifiers
struct SecretAndVerifiers {
    ByteArray Ke;  // Ключ Ke (обычно 16 байт)
    ByteArray hAY; // HMAC от Y с ключом KcA (например, 16 байт)
    ByteArray hBX; // HMAC от X с ключом KcB (например, 16 байт)
};

// =====================================================================
// Основной класс SPake2p. Он инкапсулирует всю логику протокола.
class Spake2p {
public:
    // Фабричные статические методы.

    // Вычисление w0 и w1 из PIN-кода с использованием PBKDF2.
    // pin – PIN-код (например, 4-значное число)
    // Параметр pbkdfParameters включает iterations и salt.
    static std::pair<uint256_t, uint256_t> computeW0W1(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        // Для записи PIN используем little-endian,
        // например, представим его в 4 байтах.
        ByteArray pinBytes(4);
        for (size_t i = 0; i < 4; i++) {
            pinBytes[i] = static_cast<uint8_t>((pin >> (8 * i)) & 0xff);
        }
        // Выходной буфер должен иметь длину, равную (CRYPTO_W_SIZE_BYTES * 2)
        constexpr size_t CRYPTO_W_SIZE_BYTES = 40; // пример, может быть иное, зависит от кривой (256 бит + 8)
        size_t outputLen = CRYPTO_W_SIZE_BYTES * 2;

        ByteArray ws = Crypto::pbkdf2(pinBytes, pbkdfParameters.salt, pbkdfParameters.iterations, outputLen);
        if (ws.size() < outputLen)
            throw std::runtime_error("pbkdf2: недостаточная длина вывода");

        // Преобразуем байты в число (предполагается функция bytesToBigInt)
        uint256_t w0 = bytesToBigInt(ByteArray(ws.begin(), ws.begin() + CRYPTO_W_SIZE_BYTES));
        uint256_t w1 = bytesToBigInt(ByteArray(ws.begin() + CRYPTO_W_SIZE_BYTES, ws.begin() + 2 * CRYPTO_W_SIZE_BYTES));

        // Приведение к модулю порядка кривой – предполагается, что P256_CURVE.n определён
        w0 = mod(w0, getCurveOrder());
        w1 = mod(w1, getCurveOrder());
        return { w0, w1 };
    }

    // Вычисление w0 и вычисление L = BASE * w1
    static std::pair<uint256_t, ByteArray> computeW0L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        auto [w0, w1] = computeW0W1(pbkdfParameters, pin);
        // L = BASE * w1, преобразуем в байты (не сжато)
        Point Lpoint = Point::Base().multiply(w1);
        ByteArray L = Lpoint.toBytes(false);
        return { w0, L };
    }

    // Фабричный метод, создающий экземпляр протокола.
    // context – байтовый контекст (может включать идентификаторы, и т.д.)
    // w0 – ранее вычисленный w0 (из pin)
    // Внутри генерируется случайное число random из [1, n-1].
    static Spake2p create(const ByteArray& context, uint256_t w0) {
        uint256_t random = Crypto::getRandomBigInt(32, getCurveOrder());
        return Spake2p(context, random, w0);
    }

    // Вычисление X = BASE * random + M * w0.
    // M – константная точка (определённая по стандарту, например, задана в виде массива байт)
    ByteArray computeX() const {
        Point part1 = Point::Base().multiply(random_);
        Point part2 = M().multiply(w0_);
        Point X = part1.add(part2);
        return X.toBytes(false);
    }

    // Вычисление Y = BASE * random + N * w0.
    ByteArray computeY() const {
        Point part1 = Point::Base().multiply(random_);
        Point part2 = N().multiply(w0_);
        Point Y = part1.add(part2);
        return Y.toBytes(false);
    }

    // Вычисление секретного ключа и контрольных значений (верификаторов) на основе Y-версии протокола.
    // Используется, если известен w1.
    // X и Y – обменянные байтовые представления точек.
    SecretAndVerifiers computeSecretAndVerifiersFromY(uint256_t w1,
                                                      const ByteArray& X,
                                                      const ByteArray& Y) const {
        // Преобразуем Y в точку
        Point YPoint = Point::fromBytes(Y);
        YPoint.assertValidity();
        // Вычисляем yNwo = Y – (N * w0)
        Point yNwo = YPoint.add( N().multiply(w0_).negate() );
        // Z = yNwo * random_
        Point Z = yNwo.multiply(random_);
        // V = yNwo * w1
        Point V = yNwo.multiply(w1);
        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

    // Вычисление секретного ключа и контрольных значений (верификаторов) для стороны, где известен L.
    // Используется, если известен L (другая сторона вычислила L = BASE * w1).
    SecretAndVerifiers computeSecretAndVerifiersFromX(const ByteArray& L,
                                                      const ByteArray& X,
                                                      const ByteArray& Y) const {
        Point XPoint = Point::fromBytes(X);
        Point LPoint = Point::fromBytes(L);
        XPoint.assertValidity();
        // Вычисляем: Z = (X - M*w0) * random_
        Point Z = XPoint.add(M().multiply(w0_).negate()).multiply(random_);
        // V = L * random_
        Point V = LPoint.multiply(random_);
        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

private:
    // Константы протокола. M и N – точки на кривой, константы протокола.
    // Они задаются в стандарте, здесь реализуем их как статические методы.
    static const Point& M() {
        static Point m = Point::fromBytes(hexToBytes("02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f"));
        return m;
    }
    static const Point& N() {
        static Point n = Point::fromBytes(hexToBytes("03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49"));
        return n;
    }

    // Служебный метод для вычисления секретного ключа и верификаторов.
    SecretAndVerifiers computeSecretAndVerifiers(const ByteArray& X,
                                                 const ByteArray& Y,
                                                 const ByteArray& Z,
                                                 const ByteArray& V) const {
        // Формирование транскрипта.
        ByteArray transcript = computeTranscriptHash(X, Y, Z, V);
        // Разбиение на Ka и Ke.
        // Предположим, что transcript имеет длину не менее 32 байт (например, выход SHA256)
        ByteArray Ka(transcript.begin(), transcript.begin() + 16);
        ByteArray Ke(transcript.begin() + 16, transcript.begin() + 32);

        // Генерация KcAB через HKDF: info = "ConfirmationKeys", длина 32 байт
        ByteArray info = stringToBytes("ConfirmationKeys");
        ByteArray KcAB = Crypto::hkdf(Ka, ByteArray{}, info, 32);
        // Разбиение: первые 16 байт – KcA, следующие 16 – KcB
        ByteArray KcA(KcAB.begin(), KcAB.begin() + 16);
        ByteArray KcB(KcAB.begin() + 16, KcAB.end());
        // Вычисляем HMAC ключей от данных Y и X
        SecretAndVerifiers result;
        result.Ke = Ke;
        result.hAY = Crypto::hmac(KcA, Y);
        result.hBX = Crypto::hmac(KcB, X);
        return result;
    }

    // Вычисление хэш-значения транскрипта, по которому будут определяться ключи.
    ByteArray computeTranscriptHash(const ByteArray& X,
                                    const ByteArray& Y,
                                    const ByteArray& Z,
                                    const ByteArray& V) const {
        DataWriter writer;
        // Пишем контекст (с длиной в виде UInt64 в little-endian)
        addToContext(writer, context_);
        // Далее, пустые строки (можно позже заменить на реальные значения, если требуется):
        addToContext(writer, ByteArray{});
        addToContext(writer, ByteArray{});
        // Добавляем M и N (в не сжатом формате)
        addToContext(writer, M().toBytes(false));
        addToContext(writer, N().toBytes(false));
        // Добавляем обменянные точки: X, Y, Z, V
        addToContext(writer, X);
        addToContext(writer, Y);
        addToContext(writer, Z);
        addToContext(writer, V);
        // Добавляем w0, записав его в 32 байта в big-endian (функция numberToBytesBE)
        addToContext(writer, numberToBytesBE(w0_, 32));
        // Вычисляем хэш (например, SHA256) от всего транскрипта
        return Crypto::hash(writer.toByteArray());
    }

    // Метод добавления в транскрипт – сначала записывается длина, затем данные.
    static void addToContext(DataWriter& writer, const ByteArray& data) {
        writer.writeByteArray(data);
    }

    // Приватный конструктор
    Spake2p(const ByteArray& context, uint256_t random, uint256_t w0)
        : context_(context), random_(random), w0_(w0) {}

    // Члены экземпляра
    ByteArray context_;
    uint256_t random_;
    uint256_t w0_;
};

// =====================================================================
// Пример служебных функций для конвертации
// Функция для преобразования массива байт в число (big-endian).
// Реализуйте корректно в зависимости от вашей реализации BigInt.
inline uint256_t bytesToBigInt(const ByteArray& bytes) {
    // Простой пример: интерпретировать массив байт как число (big-endian).
    uint256_t res = 0;
    for (size_t i = 0; i < bytes.size(); i++) {
        res = (res << 8) | bytes[i];
    }
    return res;
}

// Функция для приведения к модулю (a mod m)
inline uint256_t mod(uint256_t a, uint256_t m) {
    return a % m; // доработайте обработку отрицательных значений, если необходимо.
}

// Функция для преобразования числа в массив байт в big-endian с фиксированной длиной.
inline ByteArray numberToBytesBE(uint256_t num, size_t byteLen) {
    ByteArray bytes(byteLen, 0);
    for (size_t i = 0; i < byteLen; i++) {
        bytes[byteLen - 1 - i] = static_cast<uint8_t>(num & 0xff);
        num >>= 8;
    }
    return bytes;
}

// Функция для преобразования hex-строки в ByteArray.
inline ByteArray hexToBytes(const std::string& hex) {
    if (hex.size() % 2 != 0)
        throw std::invalid_argument("Неверная длина hex-строки");
    ByteArray bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Функция для преобразования строки в ByteArray (ASCII)
inline ByteArray stringToBytes(const std::string& str) {
    return ByteArray(str.begin(), str.end());
}

// Функция, возвращающая порядок (n) кривой P-256 (или secp256k1) – заменить на ваше значение.
inline uint256_t getCurveOrder() {
    // Примерное число для secp256k1 (замените на корректное)
    // N = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
    // Здесь используется заглушка; в реальной реализации используйте BigInt
    return 0xffffffffffffffffULL;
}
