#include "ui/screens.h"
#include "ui/common.h"

using namespace ui;

void DetectorScreen::buildList() {
    count_ = 0;
    for (int b = 0; b < BAND_COUNT; b++) {
        if (!(app::cfg.d.bandMask & (1 << b))) continue;
        for (int c = 0; c < CH_PER_BAND; c++) {
            uint16_t f = BAND_FREQS[b][c];
            bool dup = false;
            for (int i = 0; i < count_; i++) dup |= freqs_[i] == f;
            if (!dup) freqs_[count_++] = f;
        }
    }
    // ascending, so the ring reads like a spectrum
    for (int i = 1; i < count_; i++) {
        uint16_t v = freqs_[i];
        int j = i - 1;
        while (j >= 0 && freqs_[j] > v) freqs_[j + 1] = freqs_[j], j--;
        freqs_[j + 1] = v;
    }
}

void DetectorScreen::enter() {
    buildList();
    ring_.f0 = freqs_[0];
    ring_.f1 = freqs_[count_ - 1];
    memset(level_, 0, sizeof(level_));
    memset(active_, 0, sizeof(active_));
    idx_ = 0;
    floor_ = 0;
    havePass_ = false;
    selFreq_ = app::cfg.d.freq;
    app::rx.active().setFrequency(selFreq_);  // video receiver keeps its channel
    app::rx.scanner().setFrequency(freqs_[0]);
}

uint8_t DetectorScreen::trigger() const {
    return min(100, floor_ + app::cfg.d.threshold);
}

void DetectorScreen::computeFloor() {
    uint8_t s[CHANNEL_COUNT];
    memcpy(s, level_, count_);
    for (int i = 1; i < count_; i++) {
        uint8_t v = s[i];
        int j = i - 1;
        while (j >= 0 && s[j] > v) s[j + 1] = s[j], j--;
        s[j + 1] = v;
    }
    floor_ = s[count_ / 2];
}

int DetectorScreen::topList(uint8_t *out) const {
    bool used[CHANNEL_COUNT] = {false};
    int n = 0;
    while (n < TOP && n < count_) {
        int best = -1;
        for (int i = 0; i < count_; i++)
            if (!used[i] && (best < 0 || level_[i] > level_[best])) best = i;
        if (level_[best] <= floor_) break;
        used[best] = true;
        out[n++] = best;
    }
    return n;
}

void DetectorScreen::select(uint16_t freq) {
    selFreq_ = freq;
    app::tune(freq);
}

void DetectorScreen::tick() {
    Rx5808 &r = app::rx.scanner();
    if (!r.settled()) return;

    // fast attack, slow decay so the list does not flicker
    uint8_t v = r.readRssi();
    uint8_t &lvl = level_[idx_];
    lvl = v > lvl ? v : (lvl + v) / 2;

    if (havePass_) {
        if (!active_[idx_] && lvl >= trigger()) {
            active_[idx_] = true;
            app::buzzer.beep(3000, 120);
#if DEBUG_LOG
            Serial.printf("[DET] signal %u MHz  %u%% (floor %u%%)\n", freqs_[idx_], lvl, floor_);
#endif
        } else if (active_[idx_] && lvl + DET_HYSTERESIS < trigger()) {
            active_[idx_] = false;
        }
    }

    if (++idx_ >= count_) {
        idx_ = 0;
        computeFloor();
        havePass_ = true;
    }
    r.setFrequency(freqs_[idx_]);
}

void DetectorScreen::handle(const KeyEvent &ev) {
    if (ev.key == Key::Ok) {
        app::switchTo(Mode::Monitor);
        return;
    }
    if (ev.key != Key::Up && ev.key != Key::Down) return;

    uint8_t top[TOP];
    int n = topList(top);
    if (n == 0) return;
    int pos = -1;
    for (int i = 0; i < n; i++)
        if (freqs_[top[i]] == selFreq_) pos = i;

    if (pos < 0) pos = 0;
    else pos = (pos + (ev.key == Key::Up ? n - 1 : 1)) % n;
    select(freqs_[top[pos]]);
}

void DetectorScreen::draw(LGFX_Sprite &c) {
    ring_.drawScale(c);
    for (int i = 0; i < count_; i++) {
        uint16_t col = active_[i] ? TFT_RED : level_[i] > floor_ ? rssiColor(level_[i]) : COL_DIM;
        ring_.drawBar(c, freqs_[i], level_[i], 1.2f, col);
    }
    ring_.drawSelected(c, selFreq_);
    ring_.drawCursor(c, freqs_[idx_]);

    // strongest channels
    centerText(c, "DETECTOR", 50, &fonts::Font2, COL_DIM);
    uint8_t top[TOP];
    int n = havePass_ ? topList(top) : 0;
    if (n == 0) centerText(c, havePass_ ? "no signal" : "scanning...", 114, &fonts::Font2, COL_DIM);
    for (int i = 0; i < n; i++) {
        int ch = top[i];
        channelRow(c, i, freqs_[ch], level_[ch], freqs_[ch] == selFreq_, active_[ch]);
    }

    if (havePass_) {
        char buf[24];
        snprintf(buf, sizeof(buf), "noise %u trig %u", floor_, trigger());
        centerText(c, buf, 182, &fonts::Font0, COL_DIM);
    }
    videoFooter(c, selFreq_);
}
