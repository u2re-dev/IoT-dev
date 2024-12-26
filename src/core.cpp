#include "./main.hpp"

//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

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
bool loopTaskWDTEnabled;

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
extern "C" void app_main(void)
{
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
    Serial.begin();
#endif
#if ARDUINO_USB_MSC_ON_BOOT && !ARDUINO_USB_MODE
    MSC_Update.begin();
#endif
#if ARDUINO_USB_DFU_ON_BOOT && !ARDUINO_USB_MODE
    USB.enableDFU();
#endif
#if ARDUINO_USB_ON_BOOT && !ARDUINO_USB_MODE
    USB.begin();
#endif
    loopTaskWDTEnabled = false;

#ifdef ENABLE_ARDUINO
    initArduino();
    xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
#endif
}
