#pragma once

//
#include "../../std/types.hpp"
#include "../ecc/ecc.hpp"

//
constexpr uint8_t  H_VERSION   = 0x01;
constexpr uint16_t SER_VERSION = 0x0001;

//
constexpr size_t CRYPTO_GROUP_SIZE_BYTES = 32;
constexpr size_t CRYPTO_W_SIZE_BYTES = CRYPTO_GROUP_SIZE_BYTES + 8;
constexpr size_t PBKDF2_OUTLEN = CRYPTO_W_SIZE_BYTES * 2;

//
struct SecretAndVerifiers {
    bytes_t Ke;  // Ключ Ke (обычно 16 байт)
    bytes_t hAY; // HMAC от Y с ключом KcA (например, 16 байт)
    bytes_t hBX; // HMAC от X с ключом KcB (например, 16 байт)
};

//
struct PbkdfParameters {
    uint32_t iterations;
    bytes_t salt;
};

//
class Spake2p {
public:
    // Фабричные статические методы.

    // Вычисление w0 и w1 из PIN-кода с использованием PBKDF2.
    // pin – PIN-код (например, 4-значное число)
    // Параметр pbkdfParameters включает iterations и salt.
    static std::pair<uint256_t, uint256_t> computeW0W1(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        // Для записи PIN используем little-endian,
        // например, представим его в 4 байтах.
        bytes_t pinbytes_t(4);
        for (size_t i = 0; i < 4; i++) {
            pinbytes_t[i] = static_cast<uint8_t>((pin >> (8 * i)) & 0xff);
        }

        // Выходной буфер должен иметь длину, равную (CRYPTO_W_SIZE_BYTES * 2)
        constexpr size_t CRYPTO_W_SIZE_BYTES = 40; // пример, может быть иное, зависит от кривой (256 бит + 8)
        size_t outputLen = CRYPTO_W_SIZE_BYTES * 2;

        //
        bytes_t ws = Crypto::pbkdf2(pinbytes_t, pbkdfParameters.salt, pbkdfParameters.iterations, outputLen);
        if (ws.size() < outputLen) throw std::runtime_error("pbkdf2: недостаточная длина вывода");

        // Преобразуем байты в число (предполагается функция bytesTobigint_t)
        uint256_t w0 = bytesTobigint_t(bytes_t(ws.begin(), ws.begin() + CRYPTO_W_SIZE_BYTES));
        uint256_t w1 = bytesTobigint_t(bytes_t(ws.begin() + CRYPTO_W_SIZE_BYTES, ws.begin() + 2 * CRYPTO_W_SIZE_BYTES));

        // Приведение к модулю порядка кривой – предполагается, что P256_CURVE.n определён
        w0 = mod(w0, eccp_t::getCurveOrder());
        w1 = mod(w1, eccp_t::getCurveOrder());
        return { w0, w1 };
    }

    // Вычисление w0 и вычисление L = BASE * w1
    static std::pair<uint256_t, bytes_t> computeW0L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        auto [w0, w1] = computeW0W1(pbkdfParameters, pin);
        // L = BASE * w1, преобразуем в байты (не сжато)
        bytes_t L = (eccp_t::getBase() * w1).tobytes_t(false);
        return { w0, L };
    }

    // Фабричный метод, создающий экземпляр протокола.
    // context – байтовый контекст (может включать идентификаторы, и т.д.)
    // w0 – ранее вычисленный w0 (из pin)
    // Внутри генерируется случайное число random из [1, n-1].
    static Spake2p create(const bytes_t& context, uint256_t w0) {
        uint256_t random = Crypto::getRandombigint_t(32, getCurveOrder());
        return Spake2p(context, random, w0);
    }

    // Вычисление X = BASE * random + M * w0.
    // M – константная точка (определённая по стандарту, например, задана в виде массива байт)
    bytes_t computeX() const {
        eccp_t part1 = eccp_t::getBase() * random_;
        eccp_t part2 = eccp_t::getM()    * w0_;
        return (part1 + part2).tobytes_t(false);
    }

    // Вычисление Y = BASE * random + N * w0.
    bytes_t computeY() const {
        eccp_t part1 = eccp_t::getBase() * random_;
        eccp_t part2 = eccp_t::getN()    * w0_;
        return (part1 + part2).tobytes_t(false);
    }

    // Вычисление секретного ключа и контрольных значений (верификаторов) на основе Y-версии протокола.
    // Используется, если известен w1.
    // X и Y – обменянные байтовые представления точек.
    SecretAndVerifiers computeSecretAndVerifiersFromY(uint256_t w1, const bytes_t& X, const bytes_t& Y) const {
        // Преобразуем Y в точку
        eccp_t Yeccp_t = eccp_t::frombytes_t(Y); Yeccp_t.assertValidity();
        eccp_t yNwo = Yeccp_t - (eccp_t::getN() * w0_);
        eccp_t Z = yNwo * random_;
        eccp_t V = yNwo * w1;
        return computeSecretAndVerifiers(X, Y, Z.tobytes_t(false), V.tobytes_t(false));
    }

    // Вычисление секретного ключа и контрольных значений (верификаторов) для стороны, где известен L.
    // Используется, если известен L (другая сторона вычислила L = BASE * w1).
    SecretAndVerifiers computeSecretAndVerifiersFromX(const bytes_t& L, const bytes_t& X, const bytes_t& Y) const {
        eccp_t Xeccp_t = eccp_t::frombytes_t(X); Xeccp_t.assertValidity();
        eccp_t Leccp_t = eccp_t::frombytes_t(L);
        eccp_t Z = (Xeccp_t - (eccp_t::getM() * w0_)) * random_;
        eccp_t V =  Leccp_t * random_;
        return computeSecretAndVerifiers(X, Y, Z.tobytes_t(false), V.tobytes_t(false));
    }

private:
    // Служебный метод для вычисления секретного ключа и верификаторов.
    SecretAndVerifiers computeSecretAndVerifiers(const bytes_t& X, const bytes_t& Y, const bytes_t& Z, const bytes_t& V) const {
        // Формирование транскрипта.
        bytes_t transcript = computeTranscriptHash(X, Y, Z, V);
        // Разбиение на Ka и Ke.
        // Предположим, что transcript имеет длину не менее 32 байт (например, выход SHA256)
        bytes_t Ka(transcript.begin()     , transcript.begin() + 16);
        bytes_t Ke(transcript.begin() + 16, transcript.begin() + 32);

        // Генерация KcAB через HKDF: info = "ConfirmationKeys", длина 32 байт
        bytes_t info = stringTobytes_t("ConfirmationKeys");
        bytes_t KcAB = Crypto::hkdf(Ka, bytes_t{}, info, 32);
        // Разбиение: первые 16 байт – KcA, следующие 16 – KcB
        bytes_t KcA(KcAB.begin()     , KcAB.begin() + 16);
        bytes_t KcB(KcAB.begin() + 16, KcAB.end()       );

        // Вычисляем HMAC ключей от данных Y и X
        SecretAndVerifiers result;
        result.Ke = Ke;
        result.hAY = Crypto::hmac(KcA, Y);
        result.hBX = Crypto::hmac(KcB, X);
        return result;
    }

    // Вычисление хэш-значения транскрипта, по которому будут определяться ключи.
    bytes_t computeTranscriptHash(const bytes_t& X, const bytes_t& Y, const bytes_t& Z, const bytes_t& V) const {
        DataWriter writer;
        // Пишем контекст (с длиной в виде UInt64 в little-endian)
        addToContext(writer, context_);
        // Далее, пустые строки (можно позже заменить на реальные значения, если требуется):
        addToContext(writer, bytes_t{});
        addToContext(writer, bytes_t{});
        // Добавляем M и N (в не сжатом формате)
        addToContext(writer, M().tobytes_t(false));
        addToContext(writer, N().tobytes_t(false));
        // Добавляем обменянные точки: X, Y, Z, V
        addToContext(writer, X);
        addToContext(writer, Y);
        addToContext(writer, Z);
        addToContext(writer, V);
        // Добавляем w0, записав его в 32 байта в big-endian (функция numberTobytes_tBE)
        addToContext(writer, numberTobytes_tBE(w0_, 32));
        // Вычисляем хэш (например, SHA256) от всего транскрипта
        return Crypto::hash(writer.tobytes_t());
    }

    // Метод добавления в транскрипт – сначала записывается длина, затем данные.
    static void addToContext(DataWriter& writer, const bytes_t& data) {
        writer.writebytes_t(data);
    }

    // Приватный конструктор
    Spake2p(const bytes_t& context, uint256_t random, uint256_t w0) : context_(context), random_(random), w0_(w0) {}

    // Члены экземпляра
    bytes_t context_;
    uint256_t random_;
    uint256_t w0_;
};
