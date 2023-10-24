/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#define VERY_LARGE_STRING_LENGTH 8000

//
#include "./drivers/output/tft_display.hpp"

//
#include "./modules/tuya.hpp"
#include "./modules/http_server.hpp"

//
#include "./drivers/input/sd_card.hpp"
#include "./drivers/input/keypad_handle.hpp"

//
thread_local static JSONVar testProgramTemplate[3];
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
static TuyaDevice3 device[2] = { TuyaDevice3("dev0"), TuyaDevice3("dev1") };

//
void handleDevices() {
    if (WiFiConnected()) {
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
}

//
void setup() {
    initState();
    initDisplay();

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    nvs.begin("nvs", false);

    //
    initRTC();
    initIR([](uint32_t command){
        
        //
        if (command == 0x4) { device[CURRENT_DEVICE].tuyaInit(); } else
        if (command == 0x1) { switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2); } else
        if (command == 0x2) { switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1)); } else
        if (command == 0x3) 

        //
        //if (command == 0x43) { device[CURRENT_DEVICE].tuyaInit(); } else
        //if (command == 0x47) { switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2); } else
        //if (command == 0x45) { switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1)); } else
        //if (command == 0x9) 
        {
            switchScreen(true, CURRENT_DEVICE);

            //
            reloadConfig([](JSONVar& doc){
                device[0].loadConfig(doc["devices"][0]);
                device[1].loadConfig(doc["devices"][1]);
            });

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
            Serial.println("Trying to send command...");
            //
            JSONVar _irMap_ = device[CURRENT_DEVICE].getIRMap();
            String cmdCode = String(command, HEX);
            if (_irMap_.hasOwnProperty(cmdCode)) {
                Serial.println("Sending command...");
                if (JSON.typeof(_irMap_[cmdCode]["20"]).startsWith("boolean")) {
                    device[CURRENT_DEVICE].ENABLED = (bool)_irMap_[cmdCode]["20"];
                }
                device[CURRENT_DEVICE].sendControl(_irMap_[cmdCode]);
            }
        }
    });

    //
    switchScreen(true, CURRENT_DEVICE);

    //
    reloadConfig([](JSONVar& doc){
        device[0].loadConfig(doc["devices"][0]);
        device[1].loadConfig(doc["devices"][1]);
    });

    //
    while (LOADING_SD) {
        delay(1);
    }

    //
    initWiFi();

    //
    while (!WiFiConnected())
    {
        handleIR();
        delay(1);
    }

    //
    initServer(device);
    switchScreen(false, CURRENT_DEVICE);
    
    //
    Serial.println("Setup is done...");
}

//
void loop() {

    //
    if (INTERRUPTED.load()) {
        // show RSOD error
        if ((millis() - LAST_TIME.load()) >= STOP_TIMEOUT) {
            #ifdef ESP32
        ESP.restart();
            #else
        ESP.reset();
            #endif
        }
    } else {
        switchScreen((!CONNECTED.load() || LOADING_SD), CURRENT_DEVICE);
        handleIR();
        handleWiFi();

        //
        if (WiFiConnected()) {
            timeClient.update();
        }

        _syncTimeFn_();
        handleDevices();
        delay(1);
    }
}
