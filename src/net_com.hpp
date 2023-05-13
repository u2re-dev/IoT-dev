#pragma once

//
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

//
#include <WiFiUdp.h>

//
#include "utils.hpp"
#include "display.hpp"

//
static const size_t LIMIT = 1024;
//static char _debug_[LIMIT<<1];

//
thread_local size_t _r_length_ = 0;
thread_local size_t _s_length_ = 0;

//
thread_local uint8_t* _received_ = 0;//[LIMIT];
thread_local uint8_t* _sending_ = 0;//[LIMIT];

// TODO: async version, progressive
std::pair<size_t, uint8_t*> receive(WiFiUDP client, IPAddress const& IP_ADDRESS, size_t MINLEN = 1) {

    //
    if ((_r_length_ = client.parsePacket()) >= MINLEN && compareIP(client.remoteIP(), IP_ADDRESS)) {
        if (!_received_) { _received_ = (uint8_t*)calloc(1, LIMIT); };

        /*
        if (readLen >= LIMIT) { // SUICIDE!
            readLen = 0;
            client.stop();
            perror("CRITICAL: receive buffer overflow!");
            exit(-1);
            return;
        }*/

        client.readBytes(_received_, _r_length_);

        //binary_hex(buffer, _debug_, readLen);
        //Serial.println("Received such data: ");
        //Serial.println(String(_debug_));
    }

    return std::make_pair(_r_length_ >= MINLEN ? _r_length_ : 0, (uint8_t*)_received_);
}

// TODO: async version, progressive
std::pair<size_t, uint8_t*> receive(WiFiClient client, size_t MINLEN = 1) {

    //
    if (client.connected()) {
        _r_length_ = client.available();

        /*
        if (readLen >= LIMIT) { // SUICIDE!
            readLen = 0;
            client.stop();
            perror("CRITICAL: receive buffer overflow!");
            exit(-1);
            return;
        }*/

        if (_r_length_ >= MINLEN) {
            if (!_received_) { _received_ = (uint8_t*)calloc(1, LIMIT); };
            client.read(_received_, _r_length_);
        }

        if (_r_length_ >= MINLEN) {
            //binary_hex(buffer, _debug_, readLen);
            //Serial.println("Received such data: ");
            //Serial.println(String(_debug_));
        }
    }

    return std::make_pair(_r_length_ >= MINLEN ? _r_length_ : 0, (uint8_t*)_received_);
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




//
void send(WiFiClient client, uint8_t const* buffer, size_t readLen) {
    if (client.connected()) {
        client.flush();

        if (readLen > 0) {
            //binary_hex(buffer, _debug_, readLen);
            //Serial.println("Sending such data: ");
            //Serial.println(String(_debug_));
        }

        //
        if (readLen > 0) {
            client.write(buffer, readLen);
            //client.flush();
        }
        //for (uint16_t I=0;I<readLen;I++) {
            //client.write(buffer[I]);
        //}

        readLen = 0;
    }
}





//
void send(WiFiUDP client, IPAddress const& IP_ADDRESS, uint8_t const* buffer, size_t readLen) {
    if (readLen > 0) {
        //binary_hex(buffer, _debug_, readLen);
        //Serial.println("Sending such data: ");
        //Serial.println(String(_debug_));

        client.beginPacket(IP_ADDRESS, 6667);
        client.write(buffer, readLen);
        client.endPacket();
    }
}
