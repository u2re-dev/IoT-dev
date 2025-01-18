//
#include "./display.hpp"
#include <M5AtomDisplay.h>
#include <M5ModuleDisplay.h>
#include <M5ModuleRCA.h>
#include <M5UnitGLASS.h>
#include <M5UnitGLASS2.h>
#include <M5UnitOLED.h>
#include <M5UnitMiniOLED.h>
#include <M5UnitLCD.h>
#include <M5UnitRCA.h>
#include <M5Unified.h>

//
M5GFX& getDisplay() {
    M5.delay(1);

    //
    int display_count = M5.getDisplayCount();
    M5.setPrimaryDisplayType({
        m5::board_t::board_M5ModuleDisplay,
        m5::board_t::board_M5AtomDisplay,
    });

    //
    return M5.Displays(0);
}
