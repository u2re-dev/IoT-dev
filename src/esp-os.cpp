/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#define VERY_LARGE_STRING_LENGTH 8000

//
#include "./core/output/tft_display.hpp"
#include "./handler/device.hpp"
#include "./handler/command.hpp"
#include "./handler/fs.hpp"

//
void setup() {
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
    switchScreen(true, CURRENT_DEVICE);

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
    switchScreen(false, CURRENT_DEVICE);

    //
    Serial.println("Setup is done...");
    wakeUp();
}

//
void loop() {

    //
    if ((millis() - LAST_ACTIVE_TIME) > 10000) {
        powerSave();
    }

    //
    if (INTERRUPTED.load()) {
        // show RSOD error
        if (POWER_SAVING.load() || (millis() - LAST_TIME.load()) >= STOP_TIMEOUT) {
#ifdef ESP32
            ESP.restart();
#else
            ESP.reset();
#endif
        }
    } else {
        switchScreen((!wifi::CONNECTED.load() || LOADING_SD), CURRENT_DEVICE);

        //
        keypad::handleInput();
        wifi::handleWiFi();

        //
        if (wifi::WiFiConnected()) { rtc::timeClient.update(); }

        //
        rtc::_syncTimeFn_();

        //
        handleDevices();

        //
        delay(POWER_SAVING.load() ? 100 : 1);
    }
}
