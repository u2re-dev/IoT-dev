#pragma once

//
#define ENABLE_ARDUINO_STRING

//
#include <std/std.hpp>
#include <std/string.hpp>
#include <crypto/soc.hpp>
#include <crypto/crc.hpp>
#include <crypto/aes.hpp>
#include <crypto/PKCS7.hpp>

//
namespace tuya {

    //
    static const uint8_t prefix[4] = {0x00, 0x00, 0x55, 0xAA};
    static const uint8_t suffix[4] = {0x00, 0x00, 0xAA, 0x55};
    static const uint8_t wret = 16;

    //
    static const _String_<16> _udp_key = "yGAdlopoPVldABfn";
    static const _String_<16> _local_nonce_ = "0123456789abcdef";

    //
    class TuyaDevice3 {
    public: 

    protected: 
        bool protocol33 = false;
        _String_<22> _device_id_;
        _String_<16> _real_local_key_;

        //
        PKCS7_Padding* _padding_ = 0u;
        PKCS7_unPadding* _unpad_ = 0u;

        //
        protected:
        
        //
        uint8_t _remote_hmac_[48];
        uint8_t* _hmac_key_ = 0;

        //
        _String_<16> _remote_nonce_;
        _String_<16> _local_key_;
        _String_<1024> _store_;

        //
        uint32_t calculateSizeOfRequest(uint32_t payloadLen, bool hmac) {
            return 
                4 + // prefix
                4 + // sequence
                4 + // commandId
                4 + // length of data (+8)
                //4 + // ???
                payloadLen + 
                (hmac ? 32 : 4) + // CRC
                4;  // suffix
        }

        //
        uint8_t* encodeMessage(
            uint8_t* mem,   
            uint32_t cmdId, uint8_t const* buf, size_t& length, uint8_t* hmac
        ) {
            memcpy(mem, prefix, 4);
            *(uint32_t*)(mem + 4) = bswap32(++SEQ_NO); // TODO: sequence ID support
            *(uint32_t*)(mem + 8) = bswap32(cmdId);
            *(uint32_t*)(mem + 12) = bswap32(length+(hmac ? 32 : 4)+4);
            memcpy(mem + wret, buf, length);

            //
            if (_padding_) { freePaddingResult(_padding_); }; _padding_ = 0;
            if (hmac) {
#ifdef CONFIG_IDF_TARGET_ESP32S3
                mbedtls_md_context_t ctx;
                mbedtls_md_init(&ctx);
                mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
                mbedtls_md_hmac_starts(&ctx, (const unsigned char *) hmac, 16);
                mbedtls_md_hmac_update(&ctx, (const unsigned char *) mem, length + wret);
                mbedtls_md_hmac_finish(&ctx, mem + length + wret);
                mbedtls_md_free(&ctx);
#else
                sf_hmac_sha256(hmac, 16, mem, length + wret, mem + length + wret, 32);
#endif
            } else {
                *(uint32_t*)(mem + length + wret) = crc32_be(0, mem, length + wret);
            }
            memcpy(mem + wret + length+(hmac ? 32 : 4), suffix, 4);
            return mem;
        }

        //
        uint8_t* decodeMessage(uint32_t& cmdId, uint8_t* mem, size_t& mlen, bool hmac) {
            cmdId = bswap32(*(uint32_t*)(mem + 8));
            mlen = std::min(size_t(mlen - ((hmac ? 32 : 4) + 4 + 20)), size_t(bswap32(*(uint32_t*)(mem + 12)) - 4 - 4 - (hmac ? 32 : 4)));
            return (mem + 20);
        }

        //
        uint8_t* encryptRaw(uint8_t const* _key_, _StringView_ data, size_t& blk) {
            AES_ctx cipher;
            AES_init_ctx(&cipher, _key_);

            //
            if (_padding_) { freePaddingResult(_padding_); }; _padding_ = 0;
            _padding_ = addPadding(data.bytes(), data.length(), 16);

            for (unsigned I=0;I<_padding_->dataLengthWithPadding;I+=16) {
            AES_ECB_encrypt(&cipher, (uint8_t*)_padding_->dataWithPadding + I);
            }
            blk = _padding_->dataLengthWithPadding;
            return (uint8_t*)_padding_->dataWithPadding;
        }

        //
        uint8_t* encryptJson(uint8_t const* _key_, _StringView_ data, size_t& blk) {
            AES_ctx cipher;
            AES_init_ctx(&cipher, _key_);

            //
            size_t len_ = (data.length()+(protocol33 ? 0 : 15));
            void* data_ = calloc(1, blk = len_ + (16 - (len_ % 16)) + (protocol33 ? 15 : 0));

            //
            memcpy(data_, protocol33 ? "3.3" : "3.4", 3);
            memcpy(data_ + 15, data.bytes(), data.length());

            //
            if (_padding_) { freePaddingResult(_padding_); }; _padding_ = 0;
            _padding_ = addPadding(data_+(protocol33 ? 15 : 0), len_, 16, data_+(protocol33 ? 15 : 0));
            _padding_->dataWithPadding = data_;

            //
            for (unsigned I=0;I<_padding_->dataLengthWithPadding;I+=16) {
                AES_ECB_encrypt(&cipher, (uint8_t*)data_ + I + (protocol33 ? 15 : 0));
            }

            //
            return (uint8_t*)data_;
        }

        //
        uint8_t* decryptRaw(uint8_t* _key_, uint8_t* data, size_t& blk) {
            AES_ctx cipher;
            AES_init_ctx(&cipher, _key_);

            //
            for (unsigned I=0;I<blk;I+=16) {
            AES_ECB_decrypt(&cipher, data+I);
            }

            //
            if (_unpad_) { freeUnPaddingResult(_unpad_); }; _unpad_ = 0;
            _unpad_ = removePadding(data, blk);

            //
            blk = _unpad_->dataLengthWithoutPadding;
            if (_unpad_) { freeUnPaddingResult(_unpad_); }; _unpad_ = 0;
            return data;
        }

        //
        uint8_t* decryptJson(uint8_t* _key_, uint8_t* data, size_t& blk) {
            AES_ctx cipher;
            AES_init_ctx(&cipher, _key_);

            //
            for (unsigned I=0;I<(blk-(protocol33?15:0));I+=16) {
            AES_ECB_decrypt(&cipher, data+I+(protocol33?15:0));
            }

            //
            if (_unpad_) { freeUnPaddingResult(_unpad_); }; _unpad_ = 0;
            _unpad_ = removePadding(data+(protocol33?15:0), blk-(protocol33?15:0));
            blk = _unpad_->dataLengthWithoutPadding - (protocol33?0:15);
            if (_unpad_) { freeUnPaddingResult(_unpad_); }; _unpad_ = 0;
            return (uint8_t*)data+15;
        }

        //
        void prepareForProtocol34(uint8_t* encoded, size_t _len_) {
            AES_ctx cipher;
            AES_init_ctx(&cipher, (uint8_t*)_local_key_.bytes());
            _remote_nonce_ = (char const*)decryptRaw((uint8_t*)_local_key_.bytes(), encoded, _len_);

            //       
            size_t hlen = 48;

#ifdef CONFIG_IDF_TARGET_ESP32S3
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
            mbedtls_md_hmac_starts(&ctx, (const unsigned char *) _local_key_.bytes(), 16);
            mbedtls_md_hmac_update(&ctx, (const unsigned char *) _remote_nonce_.bytes(), 16);
            mbedtls_md_hmac_finish(&ctx, _remote_hmac_);
            mbedtls_md_free(&ctx);
#else
            sf_hmac_sha256(_local_key_.bytes(), 16, _remote_nonce_.bytes(), 16, _remote_hmac_, hlen);
#endif

            AES_ECB_encrypt(&cipher, _remote_hmac_);
            AES_ECB_encrypt(&cipher, _remote_hmac_ + 16);
            AES_ECB_encrypt(&cipher, _remote_hmac_ + 32);
            //if (!channel::_sending_) { channel::_sending_ = (uint8_t*)calloc(1, channel::LIMIT); };
            //encodeMessage((uint8_t*)channel::_sending_, 5, _remote_hmac_, hlen, _hmac_key_);

            //
            for (uint8_t I=0;I<16;I+=4) { // using 32-bit mad math
                *((uint32_t*)(_local_key_.bytes()+I)) = (*((uint32_t*)(_local_nonce_.bytes()+I)))^(*((uint32_t*)(_remote_nonce_+I)));
            }
            AES_ECB_encrypt(&cipher, (uint8_t*)_local_key_.bytes());
            memcpy(_hmac_key_, _local_key_.bytes(), 16);
        }
    };

};
