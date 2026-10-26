#pragma once
#include "app.h"
#include "channels.h"
#include "config.h"

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
class ScanScreen : public Screen {
public:
    void enter() override;
    void handle(const KeyEvent &ev) override;
    void tick() override;
    void draw(LGFX_Sprite &c) override;

private:
    static constexpr int MAX_POINTS = (FREQ_MAX - FREQ_MIN) / SCAN_STEP + 1;
    static constexpr int MAX_PEAKS = 5;
    struct Peak {
        uint16_t freq;
        uint8_t rssi;
        uint8_t index;
    };

    uint16_t freqAt(int i) const { return fmin_ + i * SCAN_STEP; }
    void findPeaks();

    uint8_t rssi_[MAX_POINTS];
    uint16_t fmin_ = 0, fmax_ = 0;
    uint16_t n_ = 0, idx_ = 0, passes_ = 0;
    Peak peaks_[MAX_PEAKS];
    uint8_t peakCount_ = 0, cursor_ = 0;
};

// Endless search over the selected bands, beeps and locks on a signal
class DetectorScreen : public Screen {
public:
    void enter() override;
    void leave() override;
    void handle(const KeyEvent &ev) override;
    void tick() override;
    void draw(LGFX_Sprite &c) override;

private:
    enum State : uint8_t { SEARCH, CONFIRM, LOCK };
    struct Hit {
        uint16_t freq;
        uint8_t rssi;
    };

    void buildList();
    void next();
    void release();
    void addHistory();
    void computeFloor();
    bool ignored(int i) const;
    uint8_t trigger() const;

    uint16_t freqs_[CHANNEL_COUNT];
    uint8_t last_[CHANNEL_COUNT];
    uint32_t skipUntil_[CHANNEL_COUNT];
    bool muted_[CHANNEL_COUNT];
    uint8_t count_ = 0, idx_ = 0;
    uint8_t floor_ = 0;
    bool havePass_ = false;

    State state_ = SEARCH;
    uint8_t lockRssi_ = 0, lockPeak_ = 0;
    uint32_t belowSince_ = 0;
    uint32_t lastRead_ = 0;

    static constexpr int HISTORY = 3;
    Hit history_[HISTORY];
    uint8_t histCount_ = 0;
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

    uint8_t calItem_ = 0;     // CAL_MIN / CAL_MAX while measuring, 0 otherwise
    uint32_t calEnd_ = 0;
    uint32_t calSum_ = 0;
    uint16_t calN_ = 0;

    char msg_[24] = {0};
    uint32_t msgUntil_ = 0;
};
