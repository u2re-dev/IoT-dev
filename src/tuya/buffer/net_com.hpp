#pragma once

//
#include <hal/network.hpp>
#include <std/std.hpp>
#include <std/utils.hpp>
#include "./channel.hpp"

//
namespace com {

    // TODO: async version, progressive
    std::pair<size_t, uint8_t*> receive(WiFiClient client, size_t MINLEN = 1) {

        //
        if (client.connected()) {
            channel::_r_length_ = client.available();

            //
            if (channel::_r_length_ >= MINLEN) {
                if (!channel::_debug_) { channel::_debug_ = (char*)calloc(1, channel::LIMIT<<1); };
                if (!channel::_received_) { channel::_received_ = (uint8_t*)calloc(1, channel::LIMIT); };
                client.read(channel::_received_, channel::_r_length_);
            }
        }

        //
        return std::make_pair(channel::_r_length_ >= MINLEN ? channel::_r_length_ : 0, (uint8_t*)channel::_received_);
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
                    return readLen;
                }
                delay(1);
            }
        }
        return readLen;
    }

    // TODO: async version, progressive
    std::pair<size_t, uint8_t*> receive(WiFiUDP client, IPAddress const& IP_ADDRESS, size_t MINLEN = 1) {

        //
        if ((channel::_r_length_ = client.parsePacket()) >= MINLEN && compareIP(client.remoteIP(), IP_ADDRESS)) {
            if (!channel::_debug_) { channel::_debug_ = (char*)calloc(1, channel::LIMIT<<1); };
            if (!channel::_received_) { channel::_received_ = (uint8_t*)calloc(1, channel::LIMIT); };

            //
            client.readBytes(channel::_received_, channel::_r_length_);

            //
            binary_hex(channel::_received_, channel::_debug_, channel::_r_length_);
        }

        return std::make_pair(channel::_r_length_ >= MINLEN ? channel::_r_length_ : 0, (uint8_t*)channel::_received_);
    }

    //
    void send(WiFiUDP client, IPAddress const& IP_ADDRESS, uint8_t const* buffer, size_t readLen) {
        if (readLen > 0) {
            client.beginPacket(IP_ADDRESS, 6667);
            client.write(buffer, readLen);
            client.endPacket();
        }
    }

};
