#include <M5Unified.hpp>
#include "./config.hpp"

//
void makeM5Config() {
    auto cfg = M5.config();
    cfg.external_display.module_display = true;  // default=true. use ModuleDisplay
    cfg.external_display.atom_display   = true;  // default=true. use AtomDisplay
    cfg.external_display.unit_glass     = false; // default=true. use UnitGLASS
    cfg.external_display.unit_glass2    = false; // default=true. use UnitGLASS2
    cfg.external_display.unit_oled      = false; // default=true. use UnitOLED
    cfg.external_display.unit_mini_oled = false; // default=true. use UnitMiniOLED
    cfg.external_display.unit_lcd       = false; // default=true. use UnitLCD
    cfg.external_display.unit_rca       = false; // default=true. use UnitRCA VideoOutput
    cfg.external_display.module_rca     = false; // default=true. use ModuleRCA VideoOutput
    cfg.external_rtc = true; 
    M5.begin(cfg);
}
