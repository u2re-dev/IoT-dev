#pragma once

//
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

//
#include "../interface/current.hpp"

namespace ir {

    //
    static IRrecv irrecv(IR_REMOTE_PIN);
    static decode_results irresults;
    
    //
    void handleInput() {
        
        //
        if (irrecv.decode(&irresults)) {
            if (irresults.value != -1LL && irresults.command != 0) {
                wakeUp();
                _LOG_(2, "Last IR: " + String(irresults.command, HEX));
                COM_HANDLER(irresults.command);
            }

            //
            irresults.command = 0;
            irresults.value = -1LL;
            irrecv.resume();  // Receive the next value
        }
    }

    //
    void initInput(std::function<void(uint32_t)> $) {
        Serial.println("Enabling IR...");
        irrecv.enableIRIn();
        COM_HANDLER = $;
    }

}
