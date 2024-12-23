#pragma once

//
#include "../core/crypto/aes.hpp"
#include <esp32/rom/crc.h>
#include <Arduino_JSON.h>
#include "mbedtls/md.h"

//
#include "../core/persistent/nv_typed.hpp"
#include "../core/persistent/nv_string.hpp"
#include "../core/interface/current.hpp"

//
#include "../core/time/timer.hpp"
#include "./net_com.hpp"

//
namespace tuya {

    //
    static const uint8_t prefix[4] = {0x00, 0x00, 0x55, 0xAA};
    static const uint8_t suffix[4] = {0x00, 0x00, 0xAA, 0x55};

    //
    static const uint8_t wret = 16;

    //
    static const _String_<16> _udp_key = "yGAdlopoPVldABfn";
    static const _String_<16> _local_nonce_ = "0123456789abcdef";

    //
    class TuyaDevice3 {
    public: 

        TuyaDevice3(char const* prefix) {
            _real_local_key_.setKey(nv::_concat_(prefix, "_lock"));
            _device_id_.setKey(nv::_concat_(prefix, "_id"));
            _IP_ADDRESS_.setKey(nv::_concat_(prefix, "_ip"));
            protocol33.setKey(nv::_concat_(prefix, "_pro3"));
            ENABLED.setKey(nv::_concat_(prefix, "_en"));
        }

    protected: 
        
        //
        WiFiClient client;
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
        nv::nv_bool ENABLED;
        //nv_t<bool> ENABLED;
        bool received = true;
        bool connected = false;
        
        //
        JSONVar irMap;
        JSONVar cState;

        //
        nv::_NvString_<22> _device_id_;
        nv::_NvString_<16> _real_local_key_;
        nv::nv_uint32_t _IP_ADDRESS_; // in reality, is packed 8x4 bits
        nv::nv_bool protocol33;

        //
        protected: 
        IPAddress IP_ADDRESS;
        
        uint8_t _remote_hmac_[48];
        uint8_t* _hmac_key_ = 0;
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
            //
            memcpy(mem, prefix, 4);
            *(uint32_t*)(mem + 4) = bswap32(++SEQ_NO); // TODO: sequence ID support
            *(uint32_t*)(mem + 8) = bswap32(cmdId);

            //
            *(uint32_t*)(mem + 12) = bswap32(length+(hmac ? 32 : 4)+4);
            memcpy(mem + wret, buf, length);

            //
            if (_padding_) { freePaddingResult(_padding_); }; _padding_ = 0;
            if (hmac) {
    //#ifndef ESP32
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
    //#else
                //
    //#endif
            } else {
    #ifdef ESP32
                *(uint32_t*)(mem + length + wret) = crc32_be(0, mem, length + wret);
    #else
                store32(mem + length + wret, bswap32(_crc32_(mem, length + wret)));
    #endif
            }
            memcpy(mem + wret + length+(hmac ? 32 : 4), suffix, 4);

            //
            //Serial.println("Encoded Message!");

            return mem;
        }

        //
        uint8_t* decodeMessage(uint32_t& cmdId, uint8_t* mem, size_t& mlen, bool hmac) {
            //SEQ_NO = bswap32(*(uint32_t*)(mem + 4));
            cmdId = bswap32(*(uint32_t*)(mem + 8));
            mlen = std::min(size_t(mlen - ((hmac ? 32 : 4) + 4 + 20)), size_t(bswap32(*(uint32_t*)(mem + 12)) - 4 - 4 - (hmac ? 32 : 4)));
            //mlen = bswap32(*(uint32_t*)(mem + 12)) - 4 - 4 - (hmac ? 32 : 4);
            //Serial.println("Length: " + String(mlen));
            return (mem + 20);
        }

        //
        uint8_t* encryptRaw(uint8_t const* _key_, _StringView_ data, size_t& blk) {
            AES_ctx cipher;
            AES_init_ctx(&cipher, _key_);

            //
            if (_padding_) { freePaddingResult(_padding_); }; _padding_ = 0;
            _padding_ = addPadding(data.c_str(), data.length(), 16);

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
            memcpy(data_ + 15, data.c_str(), data.length());

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

        //
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

                        //Serial.println("Connection to Tuya device...");
                        _LOG_(0, "Connection to Tuya device...");
                        
                        //
                        SEQ_NO = 0;
                        _local_key_ = _real_local_key_;

                        //
                        if (!protocol33) {
                            if (!_hmac_key_) {
                            _hmac_key_ = (uint8_t*)calloc(1, 16);
                            }
                            memcpy(_hmac_key_, _local_key_.c_str(), 16);
                            
                            //
                            sendMessageRaw(0x3, _local_nonce_);
                            received = true;
                        } else {
                            JSONVar _head_;
                            _head_["gwId"] = _device_id_.toString();
                            _head_["devId"] = _device_id_.toString();
                            _head_["uid"] = _device_id_.toString();
                            _head_["t"] = String(rtc::getTime());
                            sendMessageRaw(0xa, _store_ = JSON.stringify(_head_));
                            received = true;
                        }
                    } else {
                        connected = false;
                        attemp++;
                        lastAttemp = millis();
                        _LOG_(0, "Tuya connection failed!");
                    }
                } else {
                    connected = false;
                    attemp++;
                    lastAttemp = millis();
                    _LOG_(0, "Tuya connection failed!");
                }

            } else {
                _LOG_(0, "WiFI disconnected!");
            }
        }

        //
        JSONVar& getIRMap() {
            return irMap;
        }

        //
        void loadConfig(JSONVar doc) {
            _device_id_ = (char const*)doc["device_id"];
            _real_local_key_ = (char const*)doc["local_key"];
            irMap = merge(irMap, doc["ir_map"]);
            protocol33 = (String((char const*)doc["protocol"]) == String("3.3"));
            IP_ADDRESS.fromString((char const*)doc["local_ip"]);
            //for (uint8_t I=0;I<4;I++) { _IP_ADDRESS_[I] = IP_ADDRESS[I]; };
            _IP_ADDRESS_ = (uint32_t)IP_ADDRESS;
            client.stop();
        }

        //
        void reconnect() {
            if (wifi::WiFiConnected()) {
                if ((millis() - lastAttemp) >= 15000 || connected) {
                    attemp = 0;
                }

                //
                if (!client.connected() && attemp <= 3) {
                    //Serial.println("Tuya: Connection Probably Died!");
                    _LOG_(0, "Tuya connection died!");
                    connected = false;
                    attemp++;
                    lastAttemp = millis();
                    tuyaInit();
                }
            }
        }

        //
        void sendControl(JSONVar _dps_) {
            if (client.connected() && connected) {
                JSONVar _var_;
                if (!protocol33) {
                    _var_["protocol"] = 5;
                    _var_["t"] = rtc::getTime();
                    _var_["data"]["dps"] = _dps_;
                    cState = merge(cState, _dps_);
                    sendMessage(0xd, _store_ = JSON.stringify(_var_));
                    //Serial.println(_store_.toString());
                } else {
                    _var_["dps"] = _dps_;
                    _var_["t"] = String(rtc::getTime());
                    _var_["devId"] = _device_id_.toString();
                    _var_["uid"] = _device_id_.toString();
                    cState = merge(cState, _dps_);
                    sendMessage(0x7, _store_ = JSON.stringify(_var_));
                    //Serial.println(_store_.toString());
                }
            }
        }
        
        //
        void sendMessage(uint cmdId, _StringView_ _string_) {
            if (client.connected() && (received || (millis() - lastTime) >= 1000)) {
                size_t _s_length_ = _string_.length();
                uint8_t* _data_ = encryptJson((uint8_t*)_local_key_.c_str(), _string_, _s_length_);
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
                uint8_t* _data_ = encryptRaw((uint8_t*)_local_key_.c_str(), _string_, _s_length_);
                if (!channel::_sending_) { channel::_sending_ = (uint8_t*)calloc(1, channel::LIMIT); };
                if (!channel::_debug_) { channel::_debug_ = (char*)calloc(1, channel::LIMIT<<1); };
                encodeMessage((uint8_t*)channel::_sending_, /*0x0d*/cmdId, _data_, _s_length_, _hmac_key_);
                com::send(client, (uint8_t const*)channel::_sending_, calculateSizeOfRequest(_s_length_, _hmac_key_));
                lastTime = millis();
                received = false;
            }
        }

        // Perduino
        JSONVar& merge(JSONVar& dst, JSONVar src) {
            JSONVar _keys_ = src.keys();
            for (uint x = 0; x < _keys_.length(); x++) {
                //_StringView_ _key_((_keys_[x]).c_str() + 1, _keys_[x].length()-1);
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
        uint32_t handleReceive() {
            uint32_t cmdId = 0;

            //
            if (client.connected()) {
                auto [_len_, _received_] = com::receive(client, (20 + 4 + (_hmac_key_ ? 32 : 4)));
                
                if (_len_ > 0) {
                    received = true;
                    lastReceive = millis();

                    //
                    uint8_t* encoded = decodeMessage(cmdId, _received_, _len_, _hmac_key_);
                    Serial.println("RCommand: " + String(cmdId, HEX) + ", CmdLen: " + String(_len_));

                    //
                    if (_len_ > 0) {
                        if (cmdId == 0xa && protocol33) {
                            if (!connected) {
                                Serial.println("Connected to Tuya device!");
                                debug_info._LINE_[0]= "Connected to Tuya device!";
                                connected = true;
                            }
                            connected = true;
                            received = true;
                            attemp = 0;

                            //
                            _StringView_ _code_((char*)decryptRaw((uint8_t*)_local_key_.c_str(), encoded, _len_), _len_);
                            Serial.println(_code_);
                            cState = merge(cState, JSON.parse(_code_)["dps"]);
                        } else
                        if (cmdId == 0x4) {
                            AES_ctx cipher;
                            AES_init_ctx(&cipher, (uint8_t*)_local_key_.c_str());
                            _remote_nonce_ = (char const*)decryptRaw((uint8_t*)_local_key_.c_str(), encoded, _len_);
                            Serial.println("Remote Nonce: " + cString(_remote_nonce_, 16));

                            //       
                            size_t hlen = 48;

    #ifdef CONFIG_IDF_TARGET_ESP32S3
                            mbedtls_md_context_t ctx;
                            mbedtls_md_init(&ctx);
                            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
                            mbedtls_md_hmac_starts(&ctx, (const unsigned char *) _local_key_.c_str(), 16);
                            mbedtls_md_hmac_update(&ctx, (const unsigned char *) _remote_nonce_.c_str(), 16);
                            mbedtls_md_hmac_finish(&ctx, _remote_hmac_);
                            mbedtls_md_free(&ctx);
    #else
                            sf_hmac_sha256(_local_key_.c_str(), 16, _remote_nonce_.c_str(), 16, _remote_hmac_, hlen);
    #endif

                            AES_ECB_encrypt(&cipher, _remote_hmac_);
                            AES_ECB_encrypt(&cipher, _remote_hmac_ + 16);
                            AES_ECB_encrypt(&cipher, _remote_hmac_ + 32);
                            if (!channel::_sending_) { channel::_sending_ = (uint8_t*)calloc(1, channel::LIMIT); };
                            encodeMessage((uint8_t*)channel::_sending_, 5, _remote_hmac_, hlen, _hmac_key_);
                            com::send(client, (uint8_t const*)channel::_sending_, calculateSizeOfRequest(hlen, _hmac_key_));
                            //lastTime = millis();

                            //
        #ifndef ESP32
                            for (uint8_t I=0;I<16;I++) { _local_key_[I] = _local_nonce_[I]^_remote_nonce_[I]; };
        #else
                            for (uint8_t I=0;I<16;I+=4) { // using 32-bit mad math
                                *((uint32_t*)(_local_key_.c_str()+I)) = (*((uint32_t*)(_local_nonce_.c_str()+I)))^(*((uint32_t*)(_remote_nonce_+I)));
                            }
        #endif
                            AES_ECB_encrypt(&cipher, (uint8_t*)_local_key_.c_str());
                            memcpy(_hmac_key_, _local_key_.c_str(), 16);

                            //
                            if (!connected) {
                                Serial.println("Connected to Tuya device!");
                                _LOG_(0, "Connected to Tuya device!");
                                connected = true;

                                JSONVar _tmp_;
                                _tmp_["20"] = (bool)ENABLED;
                                sendControl(_tmp_);
                            }
                            connected = true;
                            received = true;
                            attemp = 0;
                            sendMessage(0x10, _store_ = "{}");
                            received = true;
                        } else 
                        if (cmdId == 0x10 && !protocol33) {
                            _StringView_ _code_((char*)decryptRaw((uint8_t*)_local_key_.c_str(), encoded, _len_), _len_);
                            Serial.println(_code_);
                            cState = merge(cState, JSON.parse(_code_)["dps"]);
                        } else 
                        if (cmdId == 0x8) {
                            _StringView_ _code_((char*)decryptJson((uint8_t*)_local_key_.c_str(), encoded, _len_), _len_);
                            Serial.println(_code_);
                            cState = merge(cState, JSON.parse(_code_)["dps"]);
                        }
                    }
                } else
                {
                    unsigned long _ctime_ = millis();
                    if ((_ctime_ - max(lastReceive, lastTime)) >= 120000 && client.connected()) {
                        client.stop();
                    }
                }
            } else {
                //Serial.println("Tuya: Connection Probably Died!");
                _LOG_(0, "Tuya connection died!");
                connected = false;
                //attemp++;
                //lastAttemp = millis();
            }

            return cmdId;
        }

    };

};
