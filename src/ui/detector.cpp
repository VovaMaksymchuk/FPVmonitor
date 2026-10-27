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
    memset(last_, 0, sizeof(last_));
    memset(skipUntil_, 0, sizeof(skipUntil_));
    memset(muted_, 0, sizeof(muted_));
    idx_ = 0;
    floor_ = 0;
    havePass_ = false;
    release();
    app::rx.active().setFrequency(app::cfg.d.freq);  // video receiver keeps its channel
    app::rx.scanner().setFrequency(freqs_[0]);
}

void DetectorScreen::leave() {
    app::buzzer.setDetect(0);
}

uint8_t DetectorScreen::trigger() const {
    return min(100, floor_ + app::cfg.d.threshold);
}

bool DetectorScreen::ignored(int i) const {
    return muted_[i] || (skipUntil_[i] && (int32_t)(millis() - skipUntil_[i]) < 0);
}

void DetectorScreen::computeFloor() {
    uint8_t s[CHANNEL_COUNT];
    memcpy(s, last_, count_);
    for (int i = 1; i < count_; i++) {
        uint8_t v = s[i];
        int j = i - 1;
        while (j >= 0 && s[j] > v) s[j + 1] = s[j], j--;
        s[j + 1] = v;
    }
    floor_ = s[count_ / 2];
}

void DetectorScreen::next() {
    if (++idx_ >= count_) {
        idx_ = 0;
        computeFloor();
        havePass_ = true;
    }
    app::rx.scanner().setFrequency(freqs_[idx_]);
}

void DetectorScreen::release() {
    state_ = SEARCH;
    belowSince_ = 0;
    app::buzzer.setDetect(0);
}

void DetectorScreen::addHistory() {
    for (int i = HISTORY - 1; i > 0; i--) history_[i] = history_[i - 1];
    history_[0] = {freqs_[idx_], lockPeak_};
    if (histCount_ < HISTORY) histCount_++;
}

void DetectorScreen::tick() {
    Rx5808 &r = app::rx.scanner();
    uint32_t now = millis();
    if (!r.settled()) return;

    uint8_t v;
    switch (state_) {
    case SEARCH:
        v = last_[idx_] = r.readRssi();
        if (havePass_ && v >= trigger() && !ignored(idx_)) {
            state_ = CONFIRM;
            r.setFrequency(freqs_[idx_]);  // restart settle, measure again
        } else {
            next();
        }
        break;

    case CONFIRM:
        v = last_[idx_] = r.readRssi();
        if (v >= trigger()) {
            state_ = LOCK;
            lockRssi_ = lockPeak_ = v;
            belowSince_ = 0;
            app::buzzer.beep(3000, 150);
#if DEBUG_LOG
            Serial.printf("[DET] signal %u MHz  %u%% (floor %u%%)\n", freqs_[idx_], v, floor_);
#endif
        } else {
            state_ = SEARCH;
            next();
        }
        break;

    case LOCK:
        if (now - lastRead_ < 50) return;
        lastRead_ = now;
        v = last_[idx_] = r.readRssi();
        lockRssi_ = (lockRssi_ * 3 + v) / 4;
        lockPeak_ = max(lockPeak_, lockRssi_);
        if (app::cfg.d.buzzer) app::buzzer.setDetect(max<uint8_t>(1, lockRssi_));

        if (lockRssi_ + DET_HYSTERESIS < trigger()) {
            if (!belowSince_) {
                belowSince_ = now;
            } else if (now - belowSince_ >= DET_LOST_MS) {
                addHistory();
                release();
                next();
            }
        } else {
            belowSince_ = 0;
        }
        break;
    }
}

void DetectorScreen::handle(const KeyEvent &ev) {
    if (state_ == LOCK) {
        switch (ev.key) {
        case Key::Up:  // skip for a while
            skipUntil_[idx_] = millis() + DET_SKIP_MS;
            addHistory();
            release();
            next();
            break;
        case Key::Down:  // mute until leaving the detector
            muted_[idx_] = true;
            addHistory();
            release();
            next();
            break;
        case Key::Ok:
            app::tune(freqs_[idx_]);
            app::switchTo(Mode::Monitor);
            break;
        default: break;
        }
        return;
    }
    if (ev.key == Key::Ok && histCount_ > 0) {
        app::tune(history_[0].freq);
        app::switchTo(Mode::Monitor);
    }
}

void DetectorScreen::draw(LGFX_Sprite &c) {
    // channel ring
    float seg = 360.0f / count_;
    for (int i = 0; i < count_; i++) {
        float a0 = -90 + i * seg + 0.8f;
        float a1 = -90 + (i + 1) * seg - 0.8f;
        uint16_t col;
        if (ignored(i)) col = lgfx::color565(60, 0, 80);
        else if (!havePass_ && i >= idx_) col = COL_BG_ARC;
        else col = last_[i] >= trigger() ? TFT_RED : last_[i] > floor_ ? rssiColor(last_[i] * 2) : COL_BG_ARC;
        c.fillArc(CX, CY, 104, 114, a0, a1, col);
    }
    float ca = -90 + idx_ * seg;
    c.fillArc(CX, CY, 115, 119, ca, ca + seg, TFT_WHITE);

    char name[3], buf[24];
    channelName(findChannel(freqs_[idx_]), name);

    if (state_ == LOCK) {
        c.fillCircle(CX, CY, 100, COL_ALARM);
        centerText(c, "SIGNAL!", 52, &fonts::Font4, TFT_WHITE);
        centerText(c, name, 96, &fonts::FreeSansBold24pt7b, TFT_WHITE);
        snprintf(buf, sizeof(buf), "%u MHz", freqs_[idx_]);
        centerText(c, buf, 136, &fonts::Font4, TFT_WHITE);
        snprintf(buf, sizeof(buf), "%u%%", lockRssi_);
        centerText(c, buf, 164, &fonts::Font4, TFT_YELLOW);
        centerText(c, "UP skip  DN mute", 188, &fonts::Font2, TFT_WHITE);
        centerText(c, "OK: watch", 205, &fonts::Font2, TFT_WHITE);
        return;
    }

    sourceBadge(c, 34);
    centerText(c, "DETECTOR", 56, &fonts::Font2, COL_DIM);
    snprintf(buf, sizeof(buf), "%s %u", name, freqs_[idx_]);
    centerText(c, buf, 84, &fonts::Font4, TFT_WHITE);
    if (havePass_) snprintf(buf, sizeof(buf), "floor %u%%  trig %u%%", floor_, trigger());
    else snprintf(buf, sizeof(buf), "first pass...");
    centerText(c, buf, 110, &fonts::Font2, COL_DIM);

    for (int i = 0; i < histCount_; i++) {
        channelName(findChannel(history_[i].freq), name);
        snprintf(buf, sizeof(buf), "%s %u  %u%%", name, history_[i].freq, history_[i].rssi);
        centerText(c, buf, 136 + i * 20, &fonts::Font2, i == 0 ? TFT_ORANGE : COL_DIM);
    }
    if (histCount_ > 0) centerText(c, "OK: watch last", 200, &fonts::Font2, COL_DIM);
}
