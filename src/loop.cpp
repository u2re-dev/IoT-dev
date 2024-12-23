#define VERY_LARGE_STRING_LENGTH 8000

//
#include <library/std.hpp>

//
#include <hal/output/tft_display.hpp>
#include <handler/device.hpp>
#include <handler/command.hpp>
#include <handler/fs.hpp>

//
std::thread displayTask;
extern "C" void IOTask() {
    while(!INTERRUPTED.load()) {
        if (keypad::pollingInput()) {
            wakeUp();
        }
        tft::drawFrame();
        delay(POWER_SAVING.load() ? 100 : 1);
    }
    tft::_RSOD_();
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
    keypad::initInput(COMHandler);
    tft::initDisplay();
    nv::storage.begin("nvs", false);

    //
    Serial.println("Making IO threads...");
    displayTask = std::thread(IOTask);

    //
    if (!fs::sd::loadConfig(FSHandler)) {
        if (!fs::internal::loadConfig(FSHandler)) {
            _STOP_EXCEPTION_();
        }
    }

    //
    if (!INTERRUPTED.load()) {
        rtc::initRTC();
        wifi::initWiFi();

        //
        while (!wifi::WiFiConnected())
        { keypad::handleInput(); delay(POWER_SAVING.load() ? 100 : 1); }
        rtc::initRTC();

        //
        Serial.println("Setup is done...");
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
    ESP.restart();
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
