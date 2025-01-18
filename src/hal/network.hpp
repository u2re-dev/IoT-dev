#pragma once

//
#include <WiFi.h>
#include <WiFiUdp.h>
#include <IPAddress.h>

//
#include <persistent/nv_string.hpp>
#include <std/debug.hpp>

//
static std::string wifi_ssid = "mozg";//nv::_NvString_<16> wifi_ssid("wifi_ssid");
static std::string wifi_pass = "n3v3rm1nd";//nv::_NvString_<16> wifi_pass("wifi_pass");

//
//int status = WL_IDLE_STATUS;
//IPAddress server(74,125,115,105);
//WiFiClient client;

//
wl_status_t connectWifi() {
    const auto start = millis();
    WiFi.mode(WIFI_STA);
    //if (WiFi.status() != WL_DISCONNECTED) { WiFi.disconnect(); };
    auto status = WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    while (status != WL_CONNECTED) {
        status = WiFi.status();
        DebugLineWithInterval(".", 100);
        if ((millis() - start) > 10000) break;
    }
    DebugLog("");
    DebugLog(status == WL_CONNECTED ? "Connection Success" : "Connection Failed");
    esp_netif_init();
    return status;
}

//
int connectToDevice(WiFiClient& client, IPAddress const& local, uint16_t port = 6668) {
    const auto start = millis();
    auto status = client.connect(local, port);
    while (!status) {
        status = client.connected();
        DebugLineWithInterval(".", 100);
        if ((millis() - start) > 10000) break;
    }
    DebugLog(status ? "Device Connection Success" : "Device Connection Failed");
    return status;
}

//
void waitAndSend(WiFiClient& client, uint8_t* data, size_t length = 0) {
    if (WiFi.status() != WL_CONNECTED) {
        DebugLog("WiFi is not connected");
        return;
    }
    const auto start = millis();

    // if not available, try to wait
    //while (!client.availableForWrite()) {
        //DebugLineWithInterval(".", 100);
        //if ((millis() - start) > 1000 || WiFi.status() != WL_CONNECTED) break;
    //}
    //DebugLog("");

    // if available, write
    //if (client.availableForWrite()) {
        client.write(data, length);
    //} else {
        //DebugLog("Sending Not Available");
    //}
}

//
bool waitForReceive(WiFiClient& client, uint8_t* data, size_t& length, size_t timeout = 8000) {
    bool done = false;

    // if not available, try to wait
    const auto start = millis();
    while (!client.available()) {
        //DebugLineWithInterval(".");
        if ((millis() - start) > timeout || WiFi.status() != WL_CONNECTED || !client.connected()) break;
    }
    //DebugLog("");

    //
    if (!client.connected()) DebugLog("Device Disconnected");
    if (WiFi.status() != WL_CONNECTED) DebugLog("Network Disconnected");

    // if is tuya package
    //const uint8_t prefix[] = {0x00, 0x00, 0x55, 0xAA};
    //const uint8_t suffix[] = {0x00, 0x00, 0xAA, 0x55};
    if (client.available()) {
        length = client.available();
        client.readBytes(data, length);
        done = true;
    }

    //
    return done;
}

//
enum ipv4_error {
    IPV4_SUCCESS = 0,
    ERROR_IPV4_DATA_OVERFLOW,
    ERROR_IPV4_INPUT_OVERFLOW,
    ERROR_IPV4_NO_SYMBOL,
    ERROR_IPV4_INVALID_SYMBOL,
    ERROR_IPV4_NOT_ENOUGH_INPUT
};

//
ipv4_error ipv4_parse( const uint8_t * string, uint8_t string_length, uint8_t* result )
{
    bool at_least_one_symbol = false;
    uint8_t symbol, string_index = 0, result_index = 0;
    uint16_t data = 0;
    while ( string_index < string_length ) {
        symbol = string[string_index];
        if ( isdigit ( symbol ) != 0 ) {
            symbol -= '0';
            data   = data * 10 + symbol;
            if ( data > UINT8_MAX ) {
                // 127.0.0.256
                return ERROR_IPV4_DATA_OVERFLOW;
            }
            at_least_one_symbol = true;
        } else if ( symbol == '.' ) {
            if ( result_index < 3 ) {
                if ( at_least_one_symbol ) {
                    result[result_index] = data;
                    data = 0;
                    result_index ++;
                    at_least_one_symbol = false;
                } else {
                    // 127.0..1
                    return ERROR_IPV4_NO_SYMBOL;
                }
            } else {
                // 127.0.0.1.2
                return ERROR_IPV4_INPUT_OVERFLOW;
            }
        } else {
            // 127.*
            return ERROR_IPV4_INVALID_SYMBOL;
        }
        string_index ++;
    }
    if ( result_index == 3 ) {
        if ( at_least_one_symbol ) {
            result[result_index] = data;
            return IPV4_SUCCESS;
        } else {
            // 127.0.0.
            return ERROR_IPV4_NOT_ENOUGH_INPUT;
        }
    } else {
        // result_index will be always less than 3
        // 127.0
        return ERROR_IPV4_NOT_ENOUGH_INPUT;
    }
}
