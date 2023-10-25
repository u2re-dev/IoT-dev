/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#define VERY_LARGE_STRING_LENGTH 8000

//
//#include <Arduino.h>

//
#include "./core/output/tft_display.hpp"
#include "./handler/device.hpp"
#include "./handler/command.hpp"
#include "./handler/fs.hpp"

//
void loop() {};
void setup() {};

//
int app_main() {
    setCpuFrequencyMhz(80);

    //
    initState();
    tft::initDisplay();

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    nv::storage.begin("nvs", false);

    //
    rtc::initRTC();
    keypad::initInput(COMHandler);

    //
    if (!fs::sd::loadConfig(FSHandler)) {
        if (!fs::internal::loadConfig(FSHandler)) {
            _STOP_EXCEPTION_();
        }
    }

    //
    wifi::initWiFi();
    while (!wifi::WiFiConnected())
    { keypad::handleInput(); delay(POWER_SAVING.load() ? 100 : 1); }

    //
    http::initServer(device);

    //
    Serial.println("Setup is done...");
    wakeUp();

    //
    while (!INTERRUPTED.load()) {
        //
        if ((millis() - LAST_ACTIVE_TIME) > 10000) {
            powerSave();
        }

        //
        {
            switchScreen((!wifi::CONNECTED.load() || LOADING_SD), CURRENT_DEVICE);

            //
            keypad::handleInput();
            wifi::handleWiFi();

            // 
            if (wifi::WiFiConnected()) { rtc::timeClient.update(); }
            rtc::_syncTimeFn_();

            //
            handleDevices();
        }

        //
        delay(POWER_SAVING.load() ? 100 : 1);
    }

    //
    _STOP_EXCEPTION_();
    while (!(POWER_SAVING.load() || (millis() - LAST_TIME.load()) >= STOP_TIMEOUT)) {
        delay(POWER_SAVING.load() ? 100 : 1);
    }

    //
#ifdef ESP32
    ESP.restart();
#else
    ESP.reset();
#endif

    return -1;
}
