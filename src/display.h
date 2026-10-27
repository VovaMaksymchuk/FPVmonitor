#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
public:
    LGFX();

private:
    lgfx::Panel_GC9A01 panel_;
    lgfx::Bus_SPI bus_;
    lgfx::Light_PWM light_;
};

extern LGFX tft;
extern LGFX_Sprite canvas;   // full-screen frame buffer

bool displayBegin();
void displayPush();
