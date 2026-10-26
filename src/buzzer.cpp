#include "buzzer.h"

void Buzzer::begin(int pin) {
    pin_ = pin;
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, LOW);
}

void Buzzer::setEnabled(bool on) {
    enabled_ = on;
    if (!on) stop();
}

void Buzzer::start(uint16_t hz) {
    tone(pin_, hz);
    on_ = true;
}

void Buzzer::stop() {
    if (!on_) return;
    noTone(pin_);
    digitalWrite(pin_, LOW);
    on_ = false;
}

void Buzzer::beep(uint16_t hz, uint16_t ms) {
    if (!enabled_ || pin_ < 0) return;
    start(hz);
    offAt_ = millis() + ms;
}

void Buzzer::update() {
    uint32_t now = millis();
    if (on_ && (int32_t)(now - offAt_) >= 0) stop();

    if (detect_ == 0 || on_) return;
    if ((int32_t)(now - nextDetectBeep_) < 0) return;

    // weak signal: slow, low beeps; strong: fast, high
    uint16_t hz = 1200 + detect_ * 25;
    uint16_t period = 700 - detect_ * 6;
    beep(hz, 40);
    nextDetectBeep_ = now + period;
}
