#define VERY_LARGE_STRING_LENGTH 8000

//
#include <library/std.hpp>
#include <hal/interface/current.hpp>
#include <hal/network/wifi.hpp>
#include <hal/network/rtc.hpp>

//
#ifdef ENABLE_ARDUINO
#include <arduino/output/tft_display.hpp>
#include <arduino/handler/fs.hpp>
#include <arduino/handler/device.hpp>
#include <arduino/handler/command.hpp>
#endif

//
std::thread displayTask;
extern "C" void IOTask() {
    while(!INTERRUPTED.load()) {
#ifdef ENABLE_ARDUINO
        if (keypad::pollingInput()) {
            wakeUp();
        }
        tft::drawFrame();
#endif
#ifdef ENABLE_TICK
        delay(POWER_SAVING.load() ? 100 : 1);
#endif
    }
#ifdef ENABLE_ARDUINO
    tft::_RSOD_();
#endif
}

//
extern "C" void loopTask(void *pvParameters)
{
#ifdef ENABLE_CTL
    setCpuFrequencyMhz(80);

    //
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);

    //
    digitalWrite(PIN_POWER_ON, LOW);
    digitalWrite(PIN_LCD_BL, LOW);
#endif

    //
    initState();

    //
#ifdef ENABLE_ARDUINO
    Serial.setDebugOutput(true);
    Serial.begin(115200);
#endif

    //
#ifdef ENABLE_ARDUINO
    keypad::initInput(COMHandler);
    tft::initDisplay();
    nv::storage.begin("nvs", false);
#endif

    //
    displayTask = std::thread(IOTask);

    //
#ifdef ENABLE_ARDUINO
    if (!fs::sd::loadConfig(FSHandler)) {
        if (!fs::internal::loadConfig(FSHandler)) {
            _STOP_EXCEPTION_();
        }
    }
#endif

    //
    if (!INTERRUPTED.load()) {
#ifdef ENABLE_ARDUINO
        rtc::initRTC();
        wifi::initWiFi();

        //
        while (!wifi::WiFiConnected()) { keypad::handleInput(); delay(POWER_SAVING.load() ? 100 : 1); }
        rtc::initRTC();
        Serial.println("Setup is done...");
#endif
    }

    //
    if (!INTERRUPTED.load()) {
        wakeUp();
    }

    //
    while (!INTERRUPTED.load()) {
        //
#ifdef ENABLE_TICK
        if ((millis() - LAST_ACTIVE_TIME) > 10000) {
            powerSave();
        }
#endif

        //
#ifdef ENABLE_ARDUINO
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
#endif

        //
#ifdef ENABLE_TICK
        delay(POWER_SAVING.load() ? 100 : 1);
#endif
    }

    //
    //_STOP_EXCEPTION_();

#ifdef ENABLE_TICK
    while (!(POWER_SAVING.load() || (millis() - LAST_TIME.load()) >= STOP_TIMEOUT)) {
        delay(POWER_SAVING.load() ? 100 : 1);
    }
#endif

    //
    //ESP.restart();
}

//
#if CONFIG_FREERTOS_UNICORE
void yieldIfNecessary(void){
    static uint64_t lastYield = 0;
    uint64_t now = millis();
    if((now - lastYield) > 2000) {
        lastYield = now;
        vTaskDelay(5); //delay 1 RTOS tick
    }
}
#endif
