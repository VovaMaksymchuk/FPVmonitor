#pragma once
#include "buttons.h"
#include "buzzer.h"
#include "display.h"
#include "receivers.h"
#include "settings.h"

class Screen {
public:
    virtual ~Screen() {}
    virtual void enter() {}            // also called when the video source switches
    virtual void leave() {}
    virtual void handle(const KeyEvent &ev) {}
    virtual void tick() {}
    virtual void draw(LGFX_Sprite &c) = 0;
};

namespace app {
extern Receivers rx;
extern Settings cfg;
extern Buzzer buzzer;

void switchTo(Mode m);
Mode mode();

// Tune the active receiver and remember the frequency as the current one.
void tune(uint16_t freq);
void applyCalibration();
}  // namespace app
