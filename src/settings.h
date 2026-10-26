#pragma once
#include <Arduino.h>

enum class Mode : uint8_t { Menu, Monitor, Scan, Detector, Settings };

struct SettingsData {
    uint16_t freq;
    uint8_t mode;          // Mode to resume after boot
    uint16_t rssiMin[2];   // per Source, mV
    uint16_t rssiMax[2];
    uint8_t threshold;     // detector, % above noise floor
    uint8_t bandMask;      // detector bands, bit per BAND_NAMES entry
    uint8_t scanFull;      // 0: 5645-5945, 1: FREQ_MIN-FREQ_MAX
    uint8_t buzzer;
};

class Settings {
public:
    void load();
    void markDirty();
    void update();   // delayed save
    void saveNow();

    SettingsData d;

private:
    void defaults();
    bool dirty_ = false;
    uint32_t dirtyAt_ = 0;
};
