#include "receivers.h"
#include "config.h"

static constexpr uint32_t SWITCH_DEBOUNCE_MS = 50;

Receivers::Receivers()
    : int_(PIN_RX_DATA, PIN_RX_CLK, PIN_RX_LE_INT, PIN_RSSI_INT),
      ext_(PIN_RX_DATA, PIN_RX_CLK, PIN_RX_LE_EXT, PIN_RSSI_EXT) {}

void Receivers::begin() {
    int_.begin();
    ext_.begin();
    pinMode(PIN_SRC_SENSE, INPUT_PULLUP);
    delay(5);
    source_ = pending_ = readSwitch();
}

Source Receivers::readSwitch() const {
    return digitalRead(PIN_SRC_SENSE) == LOW ? Source::External : Source::Internal;
}

bool Receivers::update() {
    Source now = readSwitch();
    if (now != pending_) {
        pending_ = now;
        pendingSince_ = millis();
        return false;
    }
    if (pending_ != source_ && millis() - pendingSince_ >= SWITCH_DEBOUNCE_MS) {
        source_ = pending_;
        return true;
    }
    return false;
}
