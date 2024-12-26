#define VERY_LARGE_STRING_LENGTH 8000

//
#include <std/std.hpp>
#include <std/timer.hpp>
#include <hal/current.hpp>
#include <modules/network/wifi.hpp>
#include <modules/network/rtc.hpp>

//
std::thread displayTask;
extern "C" void IOTask() {

}

//
extern "C" void loopTask(void *pvParameters)
{
    setCpuFrequencyMhz(80);

    //
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);

    //
    digitalWrite(PIN_POWER_ON, LOW);
    digitalWrite(PIN_LCD_BL, LOW);

    //
    initState();

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    nv::storage.begin("nvs", false);
    displayTask = std::thread(IOTask);

    //
    if (!INTERRUPTED.load()) {
        rtc::initRTC();
        wifi::initWiFi();
        rtc::initRTC();
    }

    //
    if (!INTERRUPTED.load()) {
        wakeUp();
    }

    //
    while (!INTERRUPTED.load()) {
        //
        if ((millis() - LAST_ACTIVE_TIME) > 10000) {
            powerSave();
        }

        //
        {
            switchScreen((!wifi::CONNECTED.load() || LOADING_SD), CURRENT_DEVICE);
            wifi::handleWiFi();

            // 
            if (wifi::WiFiConnected()) { rtc::timeClient.update(); }
            rtc::_syncTimeFn_();
        }

        //
        delay(POWER_SAVING.load() ? 100 : 1);
    }

    //
    while (!(POWER_SAVING.load() || (millis() - LAST_TIME.load()) >= STOP_TIMEOUT)) {
        delay(POWER_SAVING.load() ? 100 : 1);
    }
}
