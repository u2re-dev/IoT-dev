#pragma once

//
#include <library/std.hpp>

//
#ifdef ENABLE_ARDUINO
#include <arduino/core/network.hpp>
#include <arduino/persistent/nv_string.hpp>

//
namespace wifi {

    //
    static nv::_NvString_<15> ssid("ssid");
    static nv::_NvString_<15> password("password");
    static std::atomic<bool> CONNECTED; //= false;

    //
    static WiFiEventId_t wifiConnectHandler;
    static WiFiEventId_t wifiDisconnectHandler;

    //
    void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info)
    {
        Serial.println("Connected to Wi-Fi sucessfully.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        //
        Serial.print("RRSI: ");
        Serial.println(WiFi.RSSI());

        //
        CONNECTED = true;
    }

    //
    void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info)
    {   //
        CONNECTED = false;
        Serial.println("Disconnected from Wi-Fi, trying to connect...");

        //
        WiFi.begin(ssid, password);
        WiFi.reconnect();
    }

    bool WiFiConnected() {
        return (WiFi.status() == WL_CONNECTED);
    }

    //
    void initWiFi() {
        //Register event handlers
        Serial.println("WiFi events...");
        wifiDisconnectHandler = WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        wifiConnectHandler = WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

        //
        Serial.println("Connecting to WiFi...");
        WiFi.begin(ssid, password);
    }

    //
    void handleWiFi() {

    }

};
#endif
