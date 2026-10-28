#pragma once
#include "app.h"
#include "channels.h"
#include "config.h"
#include "ui/common.h"

class MenuScreen : public Screen {
public:
    void enter() override;
    void handle(const KeyEvent &ev) override;
    void draw(LGFX_Sprite &c) override;

private:
    uint8_t cursor_ = 0;
};

// Manual tuning: band / channel / exact frequency
class MonitorScreen : public Screen {
public:
    void enter() override;
    void handle(const KeyEvent &ev) override;
    void tick() override;
    void draw(LGFX_Sprite &c) override;

private:
    enum Field : uint8_t { BAND, CHAN, FREQ, FIELD_COUNT };
    void syncChannel();
    void tuneChannel();

    uint8_t field_ = BAND;
    ChannelRef ref_;
    uint8_t rssi_ = 0;
    uint16_t mv_ = 0;
    uint32_t lastRead_ = 0;
    uint32_t lastLog_ = 0;
};

// Repeated sweep with a spectrum graph and the strongest peaks
// One sweep with the scanner module, then the result: spectrum on the ring
// and the strongest channels; UP/DOWN switch the video receiver between them.
class ScanScreen : public Screen {
public:
    void enter() override;
    void handle(const KeyEvent &ev) override;
    void tick() override;
    void draw(LGFX_Sprite &c) override;

private:
    static constexpr int MAX_POINTS = (FREQ_MAX - FREQ_MIN) / SCAN_STEP + 1;
    struct Peak {
        uint16_t freq;   // snapped to a table channel when close
        uint8_t rssi;
    };

    uint16_t freqAt(int i) const { return ring_.f0 + i * SCAN_STEP; }
    void start();
    void findPeaks();

    ui::FreqRing ring_;
    uint8_t rssi_[MAX_POINTS];
    uint16_t n_ = 0, idx_ = 0;
    bool done_ = false;
    uint32_t startedAt_ = 0, scanMs_ = 0;
    uint8_t level_ = 0;                // noise floor + threshold
    Peak peaks_[ui::LIST_ROWS];
    uint8_t peakCount_ = 0;
    uint16_t selFreq_ = 0;             // channel the video receiver is on
};

// Scanner sweeps the selected bands non-stop; the strongest channels are
// listed and UP/DOWN switch the video receiver between them.
class DetectorScreen : public Screen {
public:
    void enter() override;
    void handle(const KeyEvent &ev) override;
    void tick() override;
    void draw(LGFX_Sprite &c) override;

private:
    static constexpr int TOP = ui::LIST_ROWS;

    void buildList();
    void computeFloor();
    uint8_t trigger() const;
    int topList(uint8_t *out) const;   // channel indexes, strongest first
    void select(uint16_t freq);

    ui::FreqRing ring_;
    uint16_t freqs_[CHANNEL_COUNT];
    uint8_t level_[CHANNEL_COUNT];     // smoothed RSSI per channel
    bool active_[CHANNEL_COUNT];       // above trigger (with hysteresis)
    uint8_t count_ = 0, idx_ = 0;
    uint8_t floor_ = 0;
    bool havePass_ = false;
    uint16_t selFreq_ = 0;             // channel the video receiver is on
};

class SettingsScreen : public Screen {
public:
    void enter() override;
    void handle(const KeyEvent &ev) override;
    void tick() override;
    void draw(LGFX_Sprite &c) override;

private:
    enum Item : uint8_t {
        THRESH,
        BAND_FIRST,
        BAND_LAST = BAND_FIRST + BAND_COUNT - 1,
        SCAN_RANGE,
        BUZZER,
        CAL_RX,
        CAL_MIN,
        CAL_MAX,
        BACK,
        ITEM_COUNT
    };

    void activate();
    void itemText(uint8_t item, char *buf, size_t len) const;
    void showMessage(const char *msg);

    uint8_t cursor_ = 0;
    bool editing_ = false;
    uint16_t mv_ = 0;
    uint32_t lastRead_ = 0;

    uint8_t calRx_ = 0;       // receiver being calibrated: 0 INT, 1 EXT, 2 scanner
    uint8_t calItem_ = 0;     // CAL_MIN / CAL_MAX while measuring, 0 otherwise
    uint32_t calEnd_ = 0;
    uint32_t calSum_ = 0;
    uint16_t calN_ = 0;

    char msg_[24] = {0};
    uint32_t msgUntil_ = 0;
};
