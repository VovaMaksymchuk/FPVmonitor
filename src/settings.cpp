#include "settings.h"
#include "config.h"
#include "channels.h"
#include <Preferences.h>

static constexpr uint8_t SETTINGS_VERSION = 2;
static const char *NS = "fpvmon";

void Settings::defaults() {
    d.freq = FREQ_DEFAULT;
    d.mode = (uint8_t)Mode::Monitor;
    for (int i = 0; i < 3; i++) {
        d.rssiMin[i] = RSSI_MV_MIN_DEF;
        d.rssiMax[i] = RSSI_MV_MAX_DEF;
    }
    d.threshold = DET_THRESHOLD_DEF;
    d.bandMask = (1 << BAND_COUNT) - 1;
    d.scanFull = 0;
    d.buzzer = 1;
}

void Settings::load() {
    defaults();
    Preferences p;
    if (!p.begin(NS, true)) return;
    if (p.getUChar("ver", 0) == SETTINGS_VERSION &&
        p.getBytesLength("data") == sizeof(d)) {
        p.getBytes("data", &d, sizeof(d));
    }
    p.end();

    d.freq = constrain(d.freq, FREQ_MIN, FREQ_MAX);
    if (d.mode > (uint8_t)Mode::Detector) d.mode = (uint8_t)Mode::Monitor;
    if (d.bandMask == 0) d.bandMask = (1 << BAND_COUNT) - 1;
}

void Settings::saveNow() {
    Preferences p;
    if (!p.begin(NS, false)) return;
    p.putUChar("ver", SETTINGS_VERSION);
    p.putBytes("data", &d, sizeof(d));
    p.end();
    dirty_ = false;
}

void Settings::markDirty() {
    dirty_ = true;
    dirtyAt_ = millis();
}

void Settings::update() {
    if (dirty_ && millis() - dirtyAt_ >= SETTINGS_SAVE_DELAY) saveNow();
}
