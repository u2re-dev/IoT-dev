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

        //
        ByteArray ws = Crypto::pbkdf2(pinBytes, pbkdfParameters.salt, pbkdfParameters.iterations, outputLen);
        if (ws.size() < outputLen) throw std::runtime_error("pbkdf2: недостаточная длина вывода");

        // Преобразуем байты в число (предполагается функция bytesToBigInt)
        uint256_t w0 = bytesToBigInt(ByteArray(ws.begin(), ws.begin() + CRYPTO_W_SIZE_BYTES));
        uint256_t w1 = bytesToBigInt(ByteArray(ws.begin() + CRYPTO_W_SIZE_BYTES, ws.begin() + 2 * CRYPTO_W_SIZE_BYTES));

        // Приведение к модулю порядка кривой – предполагается, что P256_CURVE.n определён
        w0 = mod(w0, Point::getCurveOrder());
        w1 = mod(w1, Point::getCurveOrder());
        return { w0, w1 };
    }

    // Вычисление w0 и вычисление L = BASE * w1
    static std::pair<uint256_t, ByteArray> computeW0L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        auto [w0, w1] = computeW0W1(pbkdfParameters, pin);
        // L = BASE * w1, преобразуем в байты (не сжато)
        ByteArray L = (Point::getBase() * w1).toBytes(false);
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
        Point part1 = Point::getBase() * random_;
        Point part2 = Point::getM()    * w0_;
        return (part1 + part2).toBytes(false);
    }

    // Вычисление Y = BASE * random + N * w0.
    ByteArray computeY() const {
        Point part1 = Point::getBase() * random_;
        Point part2 = Point::getN()    * w0_;
        return (part1 + part2).toBytes(false);
    }

    // Вычисление секретного ключа и контрольных значений (верификаторов) на основе Y-версии протокола.
    // Используется, если известен w1.
    // X и Y – обменянные байтовые представления точек.
    SecretAndVerifiers computeSecretAndVerifiersFromY(uint256_t w1, const ByteArray& X, const ByteArray& Y) const {
        // Преобразуем Y в точку
        Point YPoint = Point::fromBytes(Y); YPoint.assertValidity();
        Point yNwo = YPoint - (Point::getN() * w0_);
        Point Z = yNwo * random_;
        Point V = yNwo * w1;
        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

    // Вычисление секретного ключа и контрольных значений (верификаторов) для стороны, где известен L.
    // Используется, если известен L (другая сторона вычислила L = BASE * w1).
    SecretAndVerifiers computeSecretAndVerifiersFromX(const ByteArray& L, const ByteArray& X, const ByteArray& Y) const {
        Point XPoint = Point::fromBytes(X); XPoint.assertValidity();
        Point LPoint = Point::fromBytes(L);
        Point Z = (XPoint - (Point::getM() * w0_)) * random_;
        Point V =  LPoint * random_;
        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

private:
    // Служебный метод для вычисления секретного ключа и верификаторов.
    SecretAndVerifiers computeSecretAndVerifiers(const ByteArray& X, const ByteArray& Y, const ByteArray& Z, const ByteArray& V) const {
        // Формирование транскрипта.
        ByteArray transcript = computeTranscriptHash(X, Y, Z, V);
        // Разбиение на Ka и Ke.
        // Предположим, что transcript имеет длину не менее 32 байт (например, выход SHA256)
        ByteArray Ka(transcript.begin()     , transcript.begin() + 16);
        ByteArray Ke(transcript.begin() + 16, transcript.begin() + 32);

        // Генерация KcAB через HKDF: info = "ConfirmationKeys", длина 32 байт
        ByteArray info = stringToBytes("ConfirmationKeys");
        ByteArray KcAB = Crypto::hkdf(Ka, ByteArray{}, info, 32);
        // Разбиение: первые 16 байт – KcA, следующие 16 – KcB
        ByteArray KcA(KcAB.begin()     , KcAB.begin() + 16);
        ByteArray KcB(KcAB.begin() + 16, KcAB.end()       );

        // Вычисляем HMAC ключей от данных Y и X
        SecretAndVerifiers result;
        result.Ke = Ke;
        result.hAY = Crypto::hmac(KcA, Y);
        result.hBX = Crypto::hmac(KcB, X);
        return result;
    }

    // Вычисление хэш-значения транскрипта, по которому будут определяться ключи.
    ByteArray computeTranscriptHash(const ByteArray& X, const ByteArray& Y, const ByteArray& Z, const ByteArray& V) const {
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
    Spake2p(const ByteArray& context, uint256_t random, uint256_t w0) : context_(context), random_(random), w0_(w0) {}

    // Члены экземпляра
    ByteArray context_;
    uint256_t random_;
    uint256_t w0_;
};
