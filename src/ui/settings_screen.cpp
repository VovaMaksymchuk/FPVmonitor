#include "ui/screens.h"
#include "ui/common.h"

using namespace ui;

static const char *const CAL_NAMES[] = {"INT", "EXT", "SCAN"};

static constexpr int VISIBLE = 5;
static constexpr int ROW_H = 28;
static constexpr int ROW_Y0 = 78;
static constexpr uint32_t CAL_MS = 1000;
static constexpr uint16_t CAL_MIN_SPAN = 50;  // mV between min and max

void SettingsScreen::enter() {
    app::tune(app::cfg.d.freq);
    app::rx.scanner().setFrequency(app::cfg.d.freq);
    calRx_ = (uint8_t)app::rx.source();
    editing_ = false;
    calItem_ = 0;
}

void SettingsScreen::showMessage(const char *msg) {
    strncpy(msg_, msg, sizeof(msg_) - 1);
    msgUntil_ = millis() + 1500;
}

void SettingsScreen::itemText(uint8_t item, char *buf, size_t len) const {
    const SettingsData &d = app::cfg.d;
    int src = calRx_;
    if (item >= BAND_FIRST && item <= BAND_LAST) {
        int b = item - BAND_FIRST;
        snprintf(buf, len, "Band %c  %s", BAND_NAMES[b], (d.bandMask & (1 << b)) ? "ON" : "off");
        return;
    }
    switch (item) {
    case THRESH:     snprintf(buf, len, "Thresh  %u%%", d.threshold); break;
    case SCAN_RANGE: snprintf(buf, len, "Scan  %s", d.scanFull ? "5300-5999" : "5645-5945"); break;
    case BUZZER:     snprintf(buf, len, "Buzzer  %s", d.buzzer ? "ON" : "off"); break;
    case CAL_RX:     snprintf(buf, len, "Cal rx  %s", CAL_NAMES[calRx_]); break;
    case CAL_MIN:    snprintf(buf, len, "Cal min %umV", d.rssiMin[src]); break;
    case CAL_MAX:    snprintf(buf, len, "Cal max %umV", d.rssiMax[src]); break;
    case BACK:       snprintf(buf, len, "< Back"); break;
    }
}

void SettingsScreen::activate() {
    SettingsData &d = app::cfg.d;
    if (cursor_ >= BAND_FIRST && cursor_ <= BAND_LAST) {
        uint8_t bit = 1 << (cursor_ - BAND_FIRST);
        if ((d.bandMask & ~bit) == 0) {
            showMessage("need 1 band");
            return;
        }
        d.bandMask ^= bit;
        app::cfg.markDirty();
        return;
    }
    switch (cursor_) {
    case THRESH: editing_ = true; break;
    case SCAN_RANGE:
        d.scanFull = !d.scanFull;
        app::cfg.markDirty();
        break;
    case BUZZER:
        d.buzzer = !d.buzzer;
        app::buzzer.setEnabled(d.buzzer);
        app::cfg.markDirty();
        break;
    case CAL_RX: calRx_ = (calRx_ + 1) % Receivers::COUNT; break;
    case CAL_MIN:
    case CAL_MAX:
        calItem_ = cursor_;
        calEnd_ = millis() + CAL_MS;
        calSum_ = 0;
        calN_ = 0;
        break;
    case BACK: app::switchTo(Mode::Menu); break;
    }
}

void SettingsScreen::handle(const KeyEvent &ev) {
    if (calItem_) return;
    int dir = ev.key == Key::Up ? -1 : ev.key == Key::Down ? 1 : 0;

    if (editing_) {
        if (ev.key == Key::Ok) {
            editing_ = false;
        } else if (dir) {
            int step = ev.repeat > 5 ? 5 : 1;
            app::cfg.d.threshold = constrain((int)app::cfg.d.threshold - dir * step, 5, 80);
            app::cfg.markDirty();
        }
        return;
    }
    if (ev.key == Key::Ok) activate();
    else if (dir) cursor_ = (cursor_ + dir + ITEM_COUNT) % ITEM_COUNT;
}

void SettingsScreen::tick() {
    Rx5808 &r = app::rx.byIndex(calRx_);
    uint32_t now = millis();
    if (!r.settled() || now - lastRead_ < 20) return;
    lastRead_ = now;
    mv_ = r.readRssiMv();

    if (!calItem_) return;
    calSum_ += mv_;
    calN_++;
    if ((int32_t)(now - calEnd_) < 0) return;

    SettingsData &d = app::cfg.d;
    int src = calRx_;
    uint16_t avg = calN_ ? calSum_ / calN_ : 0;
    bool ok;
    if (calItem_ == CAL_MIN) {
        ok = avg + CAL_MIN_SPAN < d.rssiMax[src];
        if (ok) d.rssiMin[src] = avg;
    } else {
        ok = avg > d.rssiMin[src] + CAL_MIN_SPAN;
        if (ok) d.rssiMax[src] = avg;
    }
    calItem_ = 0;

    char buf[24];
    if (ok) {
        app::applyCalibration();
        app::cfg.markDirty();
        snprintf(buf, sizeof(buf), "saved %umV", avg);
    } else {
        snprintf(buf, sizeof(buf), "bad: %umV", avg);
    }
    showMessage(buf);
}

void SettingsScreen::draw(LGFX_Sprite &c) {
    sourceBadge(c);
    char buf[28];
    snprintf(buf, sizeof(buf), "%u MHz  %u mV", app::cfg.d.freq, mv_);
    centerText(c, buf, 46, &fonts::Font2, COL_DIM);

    int first = constrain((int)cursor_ - VISIBLE / 2, 0, ITEM_COUNT - VISIBLE);
    for (int row = 0; row < VISIBLE; row++) {
        int item = first + row;
        int y = ROW_Y0 + row * ROW_H;
        bool sel = item == cursor_;
        uint16_t fg = TFT_WHITE;
        if (sel) {
            bool hot = editing_ || calItem_;
            c.fillRoundRect(CX - 85, y - 12, 170, 24, 8, hot ? COL_EDIT : COL_SEL);
            if (hot) fg = TFT_BLACK;
        }
        itemText(item, buf, sizeof(buf));
        centerText(c, buf, y + 1, &fonts::Font2, fg);
    }

    if (calItem_) hint(c, "measuring...");
    else if ((int32_t)(millis() - msgUntil_) < 0) hint(c, msg_);
    else if (cursor_ == CAL_MIN) hint(c, "VTX off, OK");
    else if (cursor_ == CAL_MAX) hint(c, "VTX near, OK");
}
