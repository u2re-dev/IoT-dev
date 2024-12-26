//
// arduino
#include <Preferences.h>
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WString.h>

//
#include <SPIFFS.h>
#include <SPI.h>
#include <SD.h>

//
#include <Wire.h>

// display drivers
#ifdef ENABLE_DISPLAY
#include <TFT_eSPI.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayFonts.h>
#include <OLEDDisplayUi.h>
#include <SH1106.h>
#include <SH1106Spi.h>
#include <SH1106Wire.h>
#include <SSD1306.h>
#include <SSD1306I2C.h>
#include <SSD1306Spi.h>
#include <SSD1306Wire.h>
#endif
