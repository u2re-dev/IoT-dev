#pragma once

//
#include <WiFi.h>
#include <WiFiUdp.h>
#include <IPAddress.h>

//
#include "../persistent/nv_string.hpp"
#include "../std/debug.hpp"

//
static std::string wifi_ssid = "mozg";//nv::_NvString_<16> wifi_ssid("wifi_ssid");
static std::string wifi_pass = "n3v3rm1nd";//nv::_NvString_<16> wifi_pass("wifi_pass");

//
wl_status_t connectWifi();
int connectToDevice(WiFiClient& client, IPAddress const& local, uint16_t port = 6668);
void waitAndSend(WiFiClient& client, uint8_t* data, size_t length = 0);
bool waitForReceive(WiFiClient& client, uint8_t* data, size_t& length, size_t timeout = 8000);

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
ipv4_error ipv4_parse( const uint8_t * string, uint8_t string_length, uint8_t* result );
