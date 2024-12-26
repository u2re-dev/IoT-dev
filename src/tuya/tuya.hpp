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
#include "./buffer/net_com.hpp"
#include <modules/network/wifi.hpp>
#include <ArduinoJson.hpp>

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
        bool ENABLED = false;
        _String_<22> _device_id_;
        _String_<16> _real_local_key_;
        uint32_t _IP_ADDRESS_; // in reality, is packed 8x4 bits
        
        //
        WiFiClient client;
        IPAddress IP_ADDRESS;

        //
        PKCS7_Padding* _padding_ = 0u;
        PKCS7_unPadding* _unpad_ = 0u;

        //
        unsigned long lastReceive = 0;
        unsigned long lastTime = 0;
        unsigned long lastAttemp = 0;

        //
        uint32_t SEQ_NO = 0;
        uint8_t attemp = 0;

        //
        public: 
        bool received = true;
        bool connected = false;

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

    public: 

        //
        bool isConnected() { return connected && client.connected(); };
        void tuyaInit() {
            Serial.println("Initializing Tuya...");

            //
            IP_ADDRESS = IPAddress((uint32_t)_IP_ADDRESS_);

            //
            if (client.connected()) {
                client.stop();
                connected = false;
            }

            //
            if (wifi::WiFiConnected() && !client.connected()) 
            {
                if (client.connect(IP_ADDRESS, 6668)) {
                    delay(1);
                    if (client.connected()) {
                        SEQ_NO = 0;
                        memcpy(_local_key_.bytes(), _real_local_key_.bytes(), 16);

                        //
                        if (!protocol33) {
                            if (!_hmac_key_) { _hmac_key_ = (uint8_t*)calloc(1, 16); }
                            //memcpy(_hmac_key_, _local_key_.bytes(), 16);
                            //sendMessageRaw(0x3, _local_nonce_);
                            received = true;
                        } else {
                            //JSONVar _head_;
                            //_head_["gwId"] = _device_id_.toString();
                            //_head_["devId"] = _device_id_.toString();
                            //_head_["uid"] = _device_id_.toString();
                            //_head_["t"] = String(rtc::getTime());
                            //sendMessageRaw(0xa, _store_ = JSON.stringify(_head_));
                            received = true;
                        }
                    } else {
                        connected = false;
                        attemp++;
                        lastAttemp = millis();
                    }
                } else {
                    connected = false;
                    attemp++;
                    lastAttemp = millis();
                }

            }
        }

        //
        /*
        void loadConfig(JSONVar doc) {
            _device_id_ = (char const*)doc["device_id"];
            _real_local_key_ = (char const*)doc["local_key"];
            protocol33 = (String((char const*)doc["protocol"]) == String("3.3"));
            IP_ADDRESS.fromString((char const*)doc["local_ip"]);
            //for (uint8_t I=0;I<4;I++) { _IP_ADDRESS_[I] = IP_ADDRESS[I]; };
            _IP_ADDRESS_ = (uint32_t)IP_ADDRESS;
            client.stop();
        }*/

        //
        void reconnect() {
            if (wifi::WiFiConnected()) {
                if ((millis() - lastAttemp) >= 15000 || connected) {
                    attemp = 0;
                }

                //
                if (!client.connected() && attemp <= 3) {
                    connected = false;
                    attemp++;
                    lastAttemp = millis();
                    tuyaInit();
                }
            }
        }

        // Perduino
        JSONVar& merge(JSONVar& dst, JSONVar src) {
            JSONVar _keys_ = src.keys();
            for (uint x = 0; x < _keys_.length(); x++) {
                //_StringView_ _key_((_keys_[x]).bytes() + 1, _keys_[x].length()-1);
                String _key_ = (char const*)_keys_[x];
                if (JSON.typeof(src[_key_]).startsWith("object")) {
                    JSONVar _dst_ = dst[_key_];
                    _dst_ = merge(_dst_, src[_key_]);
                    dst[_key_] = (JSONVar)_dst_;
                }
                // TODO: arrays support
                if (JSON.typeof(src[_key_]).startsWith("string")) {
                    dst[_key_] = (char const*)src[_key_];
                } 
                if (JSON.typeof(src[_key_]).startsWith("number")) {
                    dst[_key_] = (int)src[_key_];
                } 
                if (JSON.typeof(src[_key_]).startsWith("boolean")) {
                    dst[_key_] = (bool)src[_key_];
                }
            }
            return dst;
        }

        //
        void sendMessage(uint cmdId, _StringView_ _string_) {
            if (client.connected() && (received || (millis() - lastTime) >= 1000)) {
                size_t _s_length_ = _string_.length();
                uint8_t* _data_ = encryptJson((uint8_t*)_local_key_.bytes(), _string_, _s_length_);
                if (!channel::_sending_) { channel::_sending_ = (uint8_t*)calloc(1, channel::LIMIT); };
                if (!channel::_debug_) { channel::_debug_ = (char*)calloc(1, channel::LIMIT<<1); };
                encodeMessage((uint8_t*)channel::_sending_, /*0x0d*/cmdId, _data_, _s_length_, _hmac_key_);
                com::send(client, (uint8_t const*)channel::_sending_, calculateSizeOfRequest(_s_length_, _hmac_key_));
                lastTime = millis();
                received = false;
            }
        }

        //
        void sendMessageRaw(uint cmdId, _StringView_ _string_) {
            if (client.connected() && (received || (millis() - lastTime) >= 1000)) {
                size_t _s_length_ = _string_.length();
                uint8_t* _data_ = encryptRaw((uint8_t*)_local_key_.bytes(), _string_, _s_length_);
                if (!channel::_sending_) { channel::_sending_ = (uint8_t*)calloc(1, channel::LIMIT); };
                if (!channel::_debug_) { channel::_debug_ = (char*)calloc(1, channel::LIMIT<<1); };
                encodeMessage((uint8_t*)channel::_sending_, /*0x0d*/cmdId, _data_, _s_length_, _hmac_key_);
                com::send(client, (uint8_t const*)channel::_sending_, calculateSizeOfRequest(_s_length_, _hmac_key_));
                lastTime = millis();
                received = false;
            }
        }

        //
        uint32_t handleReceive() {
            uint32_t cmdId = 0;

            //
            auto [_len_, _received_] = com::receive(client, (20 + 4 + (_hmac_key_ ? 32 : 4)));
            if (_len_ > 0) {
                received = true;
                lastReceive = millis();

                //
                uint8_t* encoded = decodeMessage(cmdId, _received_, _len_, _hmac_key_);

                //
                if (_len_ > 0) {
                    if (cmdId == 0xa && protocol33) {
                        connected = true;
                        received = true;
                        attemp = 0;

                        //
                        _StringView_ _code_((char*)decryptRaw((uint8_t*)_local_key_.bytes(), encoded, _len_), _len_);
                        //cState = merge(cState, JSON.parse(_code_)["dps"]);
                    } else
                    if (cmdId == 0x4) {
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
                        if (!channel::_sending_) { channel::_sending_ = (uint8_t*)calloc(1, channel::LIMIT); };
                        encodeMessage((uint8_t*)channel::_sending_, 5, _remote_hmac_, hlen, _hmac_key_);
                        com::send(client, (uint8_t const*)channel::_sending_, calculateSizeOfRequest(hlen, _hmac_key_));

                        //
                        for (uint8_t I=0;I<16;I+=4) { // using 32-bit mad math
                            *((uint32_t*)(_local_key_.bytes()+I)) = (*((uint32_t*)(_local_nonce_.bytes()+I)))^(*((uint32_t*)(_remote_nonce_+I)));
                        }
                        AES_ECB_encrypt(&cipher, (uint8_t*)_local_key_.bytes());
                        memcpy(_hmac_key_, _local_key_.bytes(), 16);

                        //
                        connected = true;
                        received = true;
                        attemp = 0;
                        //sendMessage(0x10, _store_ = "{}");
                        received = true;
                    } else 
                    if (cmdId == 0x10 && !protocol33) {
                        _StringView_ _code_((char*)decryptRaw((uint8_t*)_local_key_.bytes(), encoded, _len_), _len_);
                        //cState = merge(cState, JSON.parse(_code_)["dps"]);
                    } else 
                    if (cmdId == 0x8) {
                        _StringView_ _code_((char*)decryptJson((uint8_t*)_local_key_.bytes(), encoded, _len_), _len_);
                        //cState = merge(cState, JSON.parse(_code_)["dps"]);
                    }
                }
            } else
            {
                unsigned long _ctime_ = millis();
                if ((_ctime_ - max(lastReceive, lastTime)) >= 120000 && client.connected()) {
                    client.stop();
                }
            }

            return cmdId;
        }

    };

};
