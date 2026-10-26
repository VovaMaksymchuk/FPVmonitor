#pragma once
#include <Arduino.h>

// Non-blocking buzzer. Works with passive (pitch follows the request) and
// active buzzers (pitch is ignored by the hardware).
class Buzzer {
public:
    void begin(int pin);
    void setEnabled(bool on);

    void beep(uint16_t hz, uint16_t ms);
    void click() { beep(2500, 15); }

    // Periodic detector beeping: rate and pitch follow RSSI. 0 stops it.
    void setDetect(uint8_t rssi) { detect_ = rssi; }

    void update();

private:
    void start(uint16_t hz);
    void stop();

    int pin_ = -1;
    bool enabled_ = true;
    bool on_ = false;
    uint32_t offAt_ = 0;
    uint32_t nextDetectBeep_ = 0;
    uint8_t detect_ = 0;
};
