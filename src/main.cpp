#define VERY_LARGE_STRING_LENGTH 8000

//
#include <std/std.hpp>
#include "hal/interface.hpp"
#include "persistent/nv_typed.hpp"

//
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>

//
std::thread displayTask;
extern "C" void IOTask() {

}

//
extern "C" void loopTask(void *pvParameters)
{
    setCpuFrequencyMhz(240);

    //
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);

    //
    digitalWrite(PIN_POWER_ON, LOW);
    digitalWrite(PIN_LCD_BL, LOW);

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    nv::storage.begin("nvs", false);
    displayTask = std::thread(IOTask);
}
