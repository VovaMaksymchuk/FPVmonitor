#pragma once
#include "rx5808.h"

enum class Source : uint8_t { Internal = 0, External = 1 };

// Internal RX5808 + external module on the header. The active one is chosen
// by the physical DPDT switch, which also routes the video to the screen.
// A third RX5808 (scanner) has no video and is used by Scan and Detector.
class Receivers {
public:
    Receivers();
    void begin();

    // Returns true when the switch position changed (debounced).
    bool update();

    Source source() const { return source_; }
    const char *sourceName() const { return source_ == Source::Internal ? "INT" : "EXT"; }
    Rx5808 &active() { return rx(source_); }
    Rx5808 &rx(Source s) { return s == Source::Internal ? int_ : ext_; }
    Rx5808 &scanner() { return scan_; }

    // 0 = INT, 1 = EXT, 2 = scanner (calibration slots)
    static constexpr int COUNT = 3;
    Rx5808 &byIndex(int i) { return i == 2 ? scan_ : rx((Source)i); }

private:
    Source readSwitch() const;

    Rx5808 int_;
    Rx5808 ext_;
    Rx5808 scan_;
    Source source_ = Source::Internal;
    Source pending_ = Source::Internal;
    uint32_t pendingSince_ = 0;
};
