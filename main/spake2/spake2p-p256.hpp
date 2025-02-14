#ifndef D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE
#define D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE


//
#include "../../std/types.hpp"
#include "../../bigint/hex.hpp"
#include "../../std/types.hpp"
#include "crypto.hpp"

//
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include "../../mbedtls/ecc_point_compression.h"


//
#include "./raii/mpi.hpp"
#include "./raii/ecp.hpp"
#include "./raii/misc.hpp"



//
constexpr  uint8_t H_VERSION   = 0x01;
constexpr uint16_t SER_VERSION = 0x0001;

//
constexpr size_t CRYPTO_GROUP_SIZE_BYTES = 32;
constexpr size_t CRYPTO_W_SIZE_BYTES     = CRYPTO_GROUP_SIZE_BYTES + 8;
constexpr size_t PBKDF2_OUTLEN           = CRYPTO_W_SIZE_BYTES     * 2;

//
struct SecretAndVerifiers {
    bytes_t Ke;
    bytes_t hAY;
    bytes_t hBX;
};

//
struct PbkdfParameters {
    uint32_t iterations;
     bytes_t salt;
};

//
struct W0W1L {
    mbedtls_mpi w0;
    mbedtls_mpi w1;
    mbedtls_mpi random;
    mbedtls_ecp_point L;
};


//
void print_point(mbedtls_ecp_point const& P) {
    bytes_t Pb(65);
    size_t len = 65;
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    mbedtls_ecp_point_write_binary(&grp, &P, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, Pb.data(), Pb.size());
    std::cout << hex::b2h(Pb) << std::endl;
}




class Spake2p {
public:
    Spake2p(const bytes_t& context, const W0W1L& base) : context_(context), base_(base) {
        mbedtls_ecp_group_init(&group_);
        mbedtls_ecp_group_load(&group_, MBEDTLS_ECP_DP_SECP256R1);
    }

    ~Spake2p() {
        mbedtls_ecp_group_free(&group_);
    }

    
    static W0W1L computeW0W1L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        // Преобразуем PIN в байты
        bytes_t pinBytes = pinToBytes(pin);

        // Генерируем ключи w0 и w1 с помощью PBKDF2
        auto ws = crypto::pbkdf2(pinBytes, pbkdfParameters.salt, pbkdfParameters.iterations, CRYPTO_W_SIZE_BYTES * 2);
        if (ws.size() < CRYPTO_W_SIZE_BYTES * 2) {
            throw std::runtime_error("PBKDF2: недостаточная длина вывода");
        }

        // Разделяем результат PBKDF2 на w0 и w1
        auto slice0 = bytes_t(ws.begin(), ws.begin() + CRYPTO_W_SIZE_BYTES);
        auto slice1 = bytes_t(ws.begin() + CRYPTO_W_SIZE_BYTES, ws.end());

        // Инициализируем структуру W0W1L
        W0W1L w0w1L = {};
        initMpi(w0w1L.w0, slice0);
        initMpi(w0w1L.w1, slice1);

        // Загружаем группу эллиптической кривой
        mbedtls_ecp_group group;
        mbedtls_ecp_group_init(&group);
        mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1);

        // Умножаем w0 и w1 на порядок группы
        multiplyByGroupOrder(w0w1L.w0, group);
        multiplyByGroupOrder(w0w1L.w1, group);

        // Генерируем случайное число
        generateRandom(w0w1L.random);

        // Вычисляем точку L = w1 * G
        computeLPoint(w0w1L.L, w0w1L.w1, group);

        // Освобождаем ресурсы группы
        mbedtls_ecp_group_free(&group);
        return w0w1L;
    }

    bytes_t computeX() {
        EcpPointWrapper M(group_); loadMPoint(M.get());
        EcpPointWrapper X(group_); X.setZero();

        checkMbedtlsError(mbedtls_ecp_muladd(&group_, X.get(), &base_.random, &group_.G, &base_.w0, M.get()), "Failed to compute X");
        return pointToBytes(X.get());
    }

    bytes_t computeY() {
        EcpPointWrapper N(group_); loadMPoint(N.get());
        EcpPointWrapper Y(group_); Y.setZero();

        checkMbedtlsError(mbedtls_ecp_muladd(&group_, Y.get(), &base_.random, &group_.G, &base_.w0, N.get()), "Failed to compute X");
        return pointToBytes(Y.get());
    }



private:

    // Преобразование PIN в байты
    static bytes_t pinToBytes(uint32_t pin) {
        bytes_t pinBytes(4);
        for (size_t i = 0; i < 4; i++) {
            pinBytes[i] = static_cast<uint8_t>((pin >> (8 * i)) & 0xff);
        }
        return pinBytes;
    }

    // Инициализация mbedtls_mpi из байтов
    static void initMpi(mbedtls_mpi& mpi, const bytes_t& data) {
        mbedtls_mpi_init(&mpi);
        int ret = mbedtls_mpi_read_binary(&mpi, data.data(), data.size());
        if (ret != 0) { throw std::runtime_error("Ошибка инициализации mbedtls_mpi"); }
    }

    // Умножение mbedtls_mpi на порядок группы
    static void multiplyByGroupOrder(mbedtls_mpi& mpi, const mbedtls_ecp_group& group) {
        int ret = mbedtls_mpi_mul_mpi(&mpi, &mpi, &group.P);
        if (ret != 0) { throw std::runtime_error("Ошибка умножения на порядок группы"); }
    }

    // Генерация случайного числа
    static void generateRandom(mbedtls_mpi& random) {
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ctr_drbg_init(&ctr_drbg);

        // Инициализация генератора случайных чисел
        const char* pers = "spake2p_random";
        mbedtls_entropy_context entropy;
        mbedtls_entropy_init(&entropy);
        int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char*>(pers), strlen(pers));
        if (ret != 0) {
            throw std::runtime_error("Ошибка инициализации генератора случайных чисел");
        }

        // Генерация случайного числа
        mbedtls_mpi_init(&random);
        ret = mbedtls_mpi_fill_random(&random, 32, mbedtls_ctr_drbg_random, &ctr_drbg);
        if (ret != 0) { throw std::runtime_error("Ошибка генерации случайного числа"); }

        // Освобождение ресурсов
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
    }

    // Вычисление точки L = w1 * G
    static void computeLPoint(mbedtls_ecp_point& L, const mbedtls_mpi& w1, const mbedtls_ecp_group& group) {
        mbedtls_ecp_point_init(&L);
        int ret = mbedtls_ecp_muladd(&group, &L, &w1, &group.G, nullptr, nullptr);
        if (ret != 0) { throw std::runtime_error("Ошибка вычисления точки L"); }
    }




    //
    void loadMPoint(mbedtls_ecp_point* point) {
        bytes_t bt = hex::h2b("02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f");
        bytes_t xy(65); size_t oLen = 65;

        //
        checkMbedtlsError( mbedtls_ecp_decompress(&group_, bt.data(), bt.size(), xy.data(), &oLen, xy.size()), "Failed to decompress M point" );
        checkMbedtlsError( mbedtls_ecp_point_read_binary(&group_, point, xy.data(), xy.size()), "Failed to read M point" );
    }

    //
    void loadNPoint(mbedtls_ecp_point* point) {
        bytes_t bt = hex::h2b("03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49");
        bytes_t xy(65); size_t oLen = 65;

        //
        checkMbedtlsError( mbedtls_ecp_decompress(&group_, bt.data(), bt.size(), xy.data(), &oLen, xy.size()), "Failed to decompress N point" );
        checkMbedtlsError( mbedtls_ecp_point_read_binary(&group_, point, xy.data(), xy.size()), "Failed to read N point" );
    }





    bytes_t pointToBytes(const mbedtls_ecp_point* point) {
        bytes_t buffer(65);
        size_t len = 65;

        checkMbedtlsError(
            mbedtls_ecp_point_write_binary(&group_, point, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buffer.data(), buffer.size()),
            "Failed to write point to bytes"
        );

        return buffer;
    }

    mbedtls_ecp_group group_;
    bytes_t context_;
    W0W1L base_;
};

#endif /* D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE */
