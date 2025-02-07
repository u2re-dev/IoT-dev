#include "./device/loop.hpp"

//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <esp32-hal.h>
#include <Arduino.h>

//
#if __has_include (<esp_sntp.h>)
    #include <esp_sntp.h>
    #define SNTP_ENABLED 1
#elif __has_include (<sntp.h>)
    #include <sntp.h>
    #define SNTP_ENABLED 1
#endif

//
#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif

//
#ifndef ARDUINO_LOOP_STACK_SIZE
#ifndef CONFIG_ARDUINO_LOOP_STACK_SIZE
#define ARDUINO_LOOP_STACK_SIZE 8192
#else
#define ARDUINO_LOOP_STACK_SIZE CONFIG_ARDUINO_LOOP_STACK_SIZE
#endif
#endif

//
TaskHandle_t loopTaskHandle = NULL;

//
__attribute__((weak)) size_t getArduinoLoopTaskStackSize(void) { return ARDUINO_LOOP_STACK_SIZE; }
__attribute__((weak)) bool shouldPrintChipDebugReport(void) { return false; }

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

//
extern "C" void loopTask(void *pvParameters) {
    setup();
    for (;;) { loop(); };
    vTaskDelete(NULL);
};

//
extern "C" void app_main()
{
    initArduino();
    xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
}
