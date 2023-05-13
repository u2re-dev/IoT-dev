/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#define VERY_LARGE_STRING_LENGTH 8000

//
//#ifndef ESP32
//#define ESP32
//#endif

//
//#define ARDUINO_HTTP_SERVER_NO_BASIC_AUTH
//#include <Base64.h>
//#include <ArduinoHttpServer.h>

#include <Arduino.h>
#include <Wire.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

//
#include <SPI.h>
//#include <FS.h>
#include <SD.h>

//
//#include <DHT.h>
//#define DHTPIN D2
//#define DHTTYPE DHT11
//DHT dht(DHTPIN, DHTTYPE);
#include "tuya.hpp"
#include "display.hpp"

//
#include <ESPAsyncWebServer.h>

//
#ifdef ESP32
static const int CS_PIN = 5;
#else
static const int CS_PIN = 15;
#endif

//
static const uint16_t kRecvPin = 27;
static IRrecv irrecv(kRecvPin);
static decode_results results;
static nv_bool LOADING_SD;

//
void handleIR();

//
static AsyncWebServer server(80);



//
static TuyaDevice3 device[2] = { TuyaDevice3(&_screen_[1], "dev0"), TuyaDevice3(&_screen_[2], "dev1") };



//
static const char *filename = "/keys.json";  // <- SD library uses 8.3 filenames

//
thread_local static JSONVar testProgramTemplate[3];

//
std::function<void(TuyaDevice3&)> testPrograms[] = {
    [](TuyaDevice3& device){
        JSONVar& _temp_ = testProgramTemplate[0];
        //device.merge(_temp_, device.cState);

        //
        //_temp_["21"] = String("colour");
        //_temp_["24"] = HSV_TO_HEX_B(float(getTime() % 360), 1.0, 0.1);

        //
        //_temp_["23"] = 1000 - ((getTime()*100) % 1000);
        //device.sendControl(_temp_);
    }
};

//
bool reloadConfig() {
    bool LOADED = false;
    //LOADING_SD = true;

    //
    Serial.println("Reading from SD...");
    _screen_[0]._LINE_1_ = "Please, insert SD card with...";
    _screen_[0]._LINE_2_ = filename;

    //
    //bool loaded = false;
    if (SD.begin(CS_PIN)) {
        // Open file for writing
        Serial.println("SD connected...");
        File file = SD.open(filename, FILE_READ);
        if (!file) {
          Serial.println("Failed to read file, HALTED!");
          _screen_[0]._LINE_1_ = "Failed to read file, HALTED!";
          return false;
        }

        //
        JSONVar doc = JSON.parse(file.readString());
        if (JSON.typeof(doc) == "undefined") {
          Serial.println(F("Failed to read file, using default configuration"));
          _screen_[0]._LINE_1_ = "Wrong file, HALTED!";
          return false;
        }
        
        //
        delay(10);
        file.close();

        //
        Serial.println("Success to read JSON file...");
        _screen_[0]._LINE_1_ = "Success to read JSON file...";
        _screen_[0]._LINE_2_ = "";
        delay(10);

        //
        ssid = (char const*)doc["ssid"];
        password = (char const*)doc["password"];

        Serial.println("SSID: " + ssid.toString());
        Serial.println("Password: " + password.toString());

        //
        delay(10);

        //
        Serial.println("Configuring devices...");
        _screen_[0]._LINE_1_ = "Configuring devices...";

        //
        device[0].loadConfig(doc["devices"][0]);
        device[1].loadConfig(doc["devices"][1]);

        

        // eenforce enabled state
        /*
        device[0].setAutoProgram([](TuyaDevice3& _device_){
            JSONVar _tmp_;
            _tmp_["20"] = _device_.ENABLED;
            _device_.sendControl(_tmp_);
        }, 2000);
        device[1].setAutoProgram([](TuyaDevice3& _device_){
            JSONVar _tmp_;
            _tmp_["20"] = _device_.ENABLED;
            _device_.sendControl(_tmp_);
        }, 2000);*/

        //device[0].setAutoProgram(testPrograms[0], 2000);

        //
        delay(10);

        //
        Serial.println("Configured...");
        _screen_[0]._LINE_1_ = "Configured...";

        //
        LOADING_SD = false;
        LOADED = true;
    }

    //
    return LOADED;
}

//
void handleIR() {
    if (irrecv.decode(&results)) {
        if (results.value != -1LL && results.command != 0) {
            // print() & println() can't handle printing long longs. (uint64_t)
            //Serial.println(String(results.command, HEX));
            //Serial.println("");
            
            //
            String cmdCode = String(results.command, HEX);
            _screen_[DEBUG_SCREEN ? 0 : (CURRENT_DEVICE+1)]._LINE_3_= "LastIR: " + cmdCode;

            //
            if (results.command == 0x43) {
              device[CURRENT_DEVICE].tuyaInit();
            } else

            //
            if (results.command == 0x47) {
              switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2);
            } else

            //
            if (results.command == 0x45) {
              switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1));
            } else

            //
            if (results.command == 0x9) {
                switchScreen(true, CURRENT_DEVICE);

                //
                reloadConfig();

                //
                while (LOADING_SD) {
                #ifndef ESP32
                    ui.update();
                #endif
                    delay(1);
                }

                //
                if (!LOADING_SD) {
                    Serial.println("Connecting to WiFi...");

                    //
                    WiFi.disconnect(true);
                    WiFi.begin(ssid, password);

                    //
                #ifndef ESP32
                    while (!(WiFi.localIP().isSet() || WiFi.status() == WL_CONNECTED))
                #else
                    while (WiFi.status() != WL_CONNECTED) 
                #endif
                    {
                #ifndef ESP32
                        ui.update();
                #endif
                        delay(1);
                    }
                }

                switchScreen(false, CURRENT_DEVICE);
            } else {

                //
                JSONVar _irMap_ = device[CURRENT_DEVICE].getIRMap();
                if (_irMap_.hasOwnProperty(cmdCode)) {
                    if (JSON.typeof(_irMap_[cmdCode]["20"]).startsWith("boolean")) {
                        device[CURRENT_DEVICE].ENABLED = (bool)_irMap_[cmdCode]["20"];
                    }
                    device[CURRENT_DEVICE].sendControl(_irMap_[cmdCode]);
                }
            }
        }

        //
        results.command = 0;
        results.value = -1LL;

        //
        irrecv.resume();  // Receive the next value
    }
}

//
void setup() {
    //
    //EEPROM.begin(48);
    nvs.begin("nvs", false);

    //
    CURRENT_DEVICE = 0;
    DEBUG_SCREEN = true;

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    Wire.setClock(3400 * 1000);

    // Initialize I2C
    Wire1.setPins(21, 22);
    Wire1.setClock(3400 * 1000);
    Wire1.begin();

    //
    time_t compiled = cvt_date(__DATE__, __TIME__);
    Serial.println("Compiled Epoch: " + String(compiled));

    //
#ifdef ENABLE_DS1307
    if (ds1307.autoprobe()) {
        ds1307.init();

        //
        DS1307_INITIALIZED = true;

        //
        time_t now = ds1307_getEpoch();
        Serial.println("RTC Epoch: " + String(now));

        //
        if (compiled > now) {
            Serial.println("RTC DS1307 has wrong time, reset...");
            ds1307_setEpoch(compiled);
        }
    } else {
        DS1307_INITIALIZED = false;
    }
#endif

    //
#ifdef ESP32
    time_t inow = rtc.getEpoch();
#ifdef ENABLE_DS1307
    time_t icom = max(compiled, ds1307_getEpoch());
#else
    time_t icom = compiled;
#endif
    Serial.println("ESP32 RTC Epoch: " + String(inow));
    if (icom > inow) {
        Serial.println("Internal RTC has wrong time, reset...");
        rtc.setTime(icom);
    }
#endif

    //
    _syncTimeFn_();

    //
    Serial.println("Enabling IR...");
    irrecv.enableIRIn();

    //
    Serial.println("Init Display...");
    initDisplay();

    //
#ifdef ESP32
    Serial.println("Pinning to Core...");
    displayTask = std::thread(displayThread);
#endif

    //
    Serial.println("Switch Screen...");
    switchScreen(true, CURRENT_DEVICE);

    //
    reloadConfig();

    //
    while (LOADING_SD) {
    #ifndef ESP32
        ui.update();
    #endif
        delay(1);
    }

    //
    initWiFi();

    //
    while (!WiFiConnected())
    {
        handleIR();
#ifndef ESP32
        ui.update();
#endif
        delay(1);
    }

    //
    Serial.println("Setting timer...");

    //
    switchScreen(false, CURRENT_DEVICE);

    //
    Serial.println("Setting protocol server...");

    //
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {

        //
        JSONVar _var_; uint8_t where = 0;
        if (request->hasParam("device")) {
            where = max(min(int(request->getParam("device")->value().toInt()), 1), 0);
        }

        //
        JSONVar _dps_;
        if (request->hasParam("20")) {
            // Perduino!
            String _val_ = request->getParam("20")->value(); _val_.trim();
            if (_val_ == "true") {
                _dps_["20"] = true;
            } else 
            if (_val_ == "false") {
                _dps_["20"] = false;
            } else {
                _dps_["20"] = bool(_val_.toInt());
            }
        }

        //
        if (request->hasParam("21")) { _dps_["21"] = request->getParam("21")->value(); }
        if (request->hasParam("22")) { _dps_["22"] = uint(request->getParam("22")->value().toInt()); }
        if (request->hasParam("23")) { _dps_["23"] = uint(request->getParam("23")->value().toInt()); }

        //
        if (where < 2 && where >= 0) {
            if (request->hasParam("reconnect")) { device[where].tuyaInit(); } 
            if (request->hasParam("20") || request->hasParam("21") || request->hasParam("22") || request->hasParam("23")) {
                if (request->hasParam("20")) {
                    device[where].ENABLED = (bool)_dps_["20"];
                }
                device[where].sendControl(_dps_);
            }
            request->send(200, "text/plain", JSON.stringify(device[where].cState));
        } else {
            request->send(200, "text/plain", "{}");
        }
    });

    //
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
        /*String message;
        if (request->hasParam("message", true)) {
            message = request->getParam("message", true)->value();
        } else {
            message = "No message sent";
        }
        Serial.println(message);
        request->send(200, "text/plain", "Hello, POST: " + message);*/
        request->send(200, "text/plain", "Hello, POST: TODO");
    });

    //
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });

    //
    server.begin();

    //
    Serial.println("Setup is done...");
}

//
void loop() {
    switchScreen((!CONNECTED.load() || LOADING_SD), CURRENT_DEVICE);
    handleIR();
    handleWiFi();
    _syncTimeFn_();

    if (WiFiConnected()) {
        timeClient.update();

        //
        for (uint8_t I=0;I<2;I++) {
            device[I].reconnect();
        }
        for (uint8_t I=0;I<2;I++) {
            device[I].handleReceive();
        }
        //for (uint8_t I=0;I<2;I++) {
            //device[I].handleAutoProgram();
        //}
    }
#ifndef ESP32
    ui.update();
#endif
    delay(1);
}
