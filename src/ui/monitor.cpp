#include "ui/screens.h"
#include "ui/common.h"

using namespace ui;

static const char *FIELD_NAMES[] = {"BAND", "CHANNEL", "FREQ"};

void MonitorScreen::enter() {
    app::tune(app::cfg.d.freq);
    syncChannel();
    rssi_ = 0;
}

void MonitorScreen::syncChannel() {
    ref_ = findChannel(app::cfg.d.freq);
}

void MonitorScreen::tuneChannel() {
    app::tune(BAND_FREQS[ref_.band][ref_.ch]);
}

void MonitorScreen::handle(const KeyEvent &ev) {
    if (ev.key == Key::Ok) {
        field_ = (field_ + 1) % FIELD_COUNT;
        return;
    }
    if (ev.key != Key::Up && ev.key != Key::Down) return;
    int dir = ev.key == Key::Up ? 1 : -1;

    switch (field_) {
    case BAND:
        if (!ref_.valid()) {
            ref_.band = 0;
            ref_.ch = 0;
        } else {
            ref_.band = (ref_.band + dir + BAND_COUNT) % BAND_COUNT;
        }
        tuneChannel();
        break;
    case CHAN:
        if (!ref_.valid()) {
            ref_.band = 4;  // R
            ref_.ch = 0;
        } else {
            ref_.ch = (ref_.ch + dir + CH_PER_BAND) % CH_PER_BAND;
        }
        tuneChannel();
        break;
    case FREQ: {
        int step = ev.repeat < 8 ? 1 : ev.repeat < 25 ? 5 : 10;
        int f = constrain((int)app::cfg.d.freq + dir * step, FREQ_MIN, FREQ_MAX);
        app::tune(f);
        syncChannel();
        break;
    }
    }
}

void MonitorScreen::tick() {
    Rx5808 &r = app::rx.active();
    uint32_t now = millis();
    if (!r.settled() || now - lastRead_ < 50) return;
    lastRead_ = now;
    mv_ = r.readRssiMv();
    rssi_ = (rssi_ * 3 + r.mvToPercent(mv_)) / 4;

#if DEBUG_LOG
    if (now - lastLog_ >= 1000) {
        lastLog_ = now;
        Serial.printf("[%s] %u MHz  rssi %u mV  %u%%\n", app::rx.sourceName(), r.frequency(), mv_, rssi_);
    }
#endif
}

void MonitorScreen::draw(LGFX_Sprite &c) {
    rssiArc(c, rssi_);
    sourceBadge(c);

    // band and channel boxes
    char buf[16];
    const int y = 62;
    const int bx[2] = {CX - 28, CX + 28};
    for (int i = 0; i < 2; i++) {
        bool sel = field_ == (i == 0 ? BAND : CHAN);
        if (sel) c.fillRoundRect(bx[i] - 22, y - 18, 44, 36, 8, COL_EDIT);
        if (!ref_.valid()) {
            strcpy(buf, "-");
        } else if (i == 0) {
            buf[0] = BAND_NAMES[ref_.band];
            buf[1] = 0;
        } else {
            buf[0] = '1' + ref_.ch;
            buf[1] = 0;
        }
        text(c, buf, bx[i], y + 2, &fonts::FreeSansBold18pt7b, sel ? TFT_BLACK : TFT_WHITE);
    }

    // frequency
    snprintf(buf, sizeof(buf), "%u", app::cfg.d.freq);
    bool selFreq = field_ == FREQ;
    centerText(c, buf, 118, &fonts::Font7, selFreq ? COL_EDIT : TFT_WHITE);
    if (selFreq) c.fillRect(CX - 56, 146, 112, 3, COL_EDIT);
    centerText(c, "MHz", 160, &fonts::Font2, COL_DIM);

    // RSSI
    snprintf(buf, sizeof(buf), "%u%%", rssi_);
    centerText(c, buf, 186, &fonts::Font4, rssiColor(rssi_));

    hint(c, FIELD_NAMES[field_]);
}
