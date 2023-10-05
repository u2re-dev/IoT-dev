#pragma once

//
#include "wifi.hpp"
#include "../utils/utils.hpp"
#include "../graphics/screen.hpp"

//
static const size_t LIMIT = 1024;
thread_local char* _debug_ = 0;

//
thread_local size_t _r_length_ = 0;
thread_local size_t _s_length_ = 0;

//
thread_local uint8_t* _received_ = 0;//[LIMIT];
thread_local uint8_t* _sending_ = 0;//[LIMIT];

// TODO: async version, progressive
std::pair<size_t, uint8_t*> receive(WiFiClient client, size_t MINLEN = 1) {

    //
    if (client.connected()) {
        _r_length_ = client.available();

        //
        if (_r_length_ >= MINLEN) {
            if (!_debug_) { _debug_ = (char*)calloc(1, LIMIT<<1); };
            if (!_received_) { _received_ = (uint8_t*)calloc(1, LIMIT); };
            client.read(_received_, _r_length_);
        }
    }

    //
    return std::make_pair(_r_length_ >= MINLEN ? _r_length_ : 0, (uint8_t*)_received_);
}

//
void send(WiFiClient client, uint8_t const* buffer, size_t readLen) {
    if (client.connected()) {
        client.flush();

        //
        if (readLen > 0) {
            client.write(buffer, readLen);
        }

        //
        readLen = 0;
    }
}

//
size_t waitToReceiveShort(WiFiClient client, size_t& readLen) {
    unsigned long beginTime = millis();
    if (client.connected()) {
        client.flush();
        while (client.connected() && (readLen = client.available()) <= 0) {
            unsigned long time = millis();
            if ((time - beginTime) >= 15000) {
                _screen_[0]._LINE_3_ = "";
                return readLen;
            }
            _screen_[0]._LINE_3_ = "Trying to recieve package...";
            delay(1);
        }
        _screen_[0]._LINE_3_ = "";
    }
    return readLen;
}

// TODO: async version, progressive
std::pair<size_t, uint8_t*> receive(WiFiUDP client, IPAddress const& IP_ADDRESS, size_t MINLEN = 1) {

    //
    if ((_r_length_ = client.parsePacket()) >= MINLEN && compareIP(client.remoteIP(), IP_ADDRESS)) {
        if (!_debug_) { _debug_ = (char*)calloc(1, LIMIT<<1); };
        if (!_received_) { _received_ = (uint8_t*)calloc(1, LIMIT); };

        //
        client.readBytes(_received_, _r_length_);

        //
        binary_hex(_received_, _debug_, _r_length_);
        Serial.println("Received such data: ");
        Serial.println(String(_debug_));
    }

    return std::make_pair(_r_length_ >= MINLEN ? _r_length_ : 0, (uint8_t*)_received_);
}

//
void send(WiFiUDP client, IPAddress const& IP_ADDRESS, uint8_t const* buffer, size_t readLen) {
    if (readLen > 0) {
        client.beginPacket(IP_ADDRESS, 6667);
        client.write(buffer, readLen);
        client.endPacket();
    }
}
