#pragma once

//
#include <WiFiUdp.h>
#include <IPAddress.h>

//
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

//
#include "../persistent/nv_string.hpp"

//
namespace wifi {

    //
    static nv::_NvString_<15> ssid("ssid");
    static nv::_NvString_<15> password("password");
    static std::atomic<bool> CONNECTED; //= false;

    //
    #ifdef ESP32
    static WiFiEventId_t wifiConnectHandler;
    static WiFiEventId_t wifiDisconnectHandler;
    #else
    static WiFiEventHandler wifiConnectHandler;
    static WiFiEventHandler wifiDisconnectHandler;
    #endif

    //
    #ifndef ESP32
    void onWifiConnect(const WiFiEventStationModeGotIP& event) 
    #else
    void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info)
    #endif
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
    #ifndef ESP32
    void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) 
    #else
    void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info)
    #endif
    {   //
        CONNECTED = false;
        Serial.println("Disconnected from Wi-Fi, trying to connect...");

        //
        WiFi.begin(ssid, password);
        WiFi.reconnect();
    }

    bool WiFiConnected() {
    #ifndef ESP32
        return (WiFi.localIP().isSet() || WiFi.status() == WL_CONNECTED);
    #else
        return (WiFi.status() == WL_CONNECTED);
    #endif
    }

    //
    void initWiFi() {
    #ifndef ESP32
        Serial.println("Workaround WiFi...");
        WiFi.persistent(false);
        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(false);
        WiFi.printDiag(Serial);
        WiFi.mode(WIFI_OFF); //workaround
        WiFi.mode(WIFI_STA);

        //
        WiFi.disconnect(true);
        WiFi.enableSTA(true);
    #endif

        //Register event handlers
        Serial.println("WiFi events...");
    #ifndef ESP32
        wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
        wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
    #else
        wifiDisconnectHandler = WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        wifiConnectHandler = WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    #endif

        //
        Serial.println("Connecting to WiFi...");
        WiFi.begin(ssid, password);
    }

    //
    void handleWiFi() {

    }

}