#pragma once

//
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

//
#include <WiFiUdp.h>
#include <IPAddress.h>

//
#include "screen.hpp"

//
static _NvString_<15> ssid("ssid");
static _NvString_<15> password("password");
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
    _screen_[0]._LINE_1_= "Connected to WiFi!";
    _screen_[0]._LINE_2_= WiFi.localIP().toString();

    //
    Serial.print("RRSI: ");
    Serial.println(WiFi.RSSI());

    //
    //WiFi.persistent(true);
    //WiFi.setAutoConnect(true);
    //WiFi.setAutoReconnect(true);

    //ATOMIC() {
    //
    //switchScreen(false, CURRENT_DEVICE);
    //server.begin();
    CONNECTED = true;
    //}
}

//
#ifndef ESP32
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) 
#else
void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info)
#endif
{
    //server.end();

    //
    //ATOMIC() {
        CONNECTED = false;
        _screen_[0]._LINE_2_= "Disconnected...";
        Serial.println("Disconnected from Wi-Fi, trying to connect...");
        //switchScreen(true, CURRENT_DEVICE);
    //}
    //server.end();

    

    //
    //WiFi.disconnect(true);
    WiFi.begin(ssid, password);
    WiFi.reconnect();
    
    //WiFi.disconnect(true);
    //initWiFi();
    //WiFi.reconnect();
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
    //Serial.println("WiFi events...");
    //WiFi.onEvent(WiFiEvent);
    //WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

    wifiDisconnectHandler = WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    wifiConnectHandler = WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
#endif

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    
    //WiFi.reconnect();
    
    //
    /*while (!WiFiConnected())
    {
        //handleIR();
#ifndef ESP32
        ui.update();
#endif
        delay(1);
    }*/
}

//
void handleWiFi() {

}


/*
#ifdef ESP32
void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY: 
            Serial.println("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Serial.println("Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.println("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Serial.println("Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("Lost IP address and IP address is reset to 0");
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("WiFi access point  stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.println("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.println("Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Serial.println("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Serial.println("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Serial.println("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.println("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Serial.println("Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            Serial.println("Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("Ethernet disconnected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.println("Obtained IP address");
            break;
        default: break;
    }}
#endif
*/

