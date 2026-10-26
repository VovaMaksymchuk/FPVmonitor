#include "ui/screens.h"
#include "ui/common.h"

using namespace ui;

static constexpr int GX0 = 28, GW = 184;   // graph area
static constexpr int GY1 = 150, GH = 80;
static constexpr int PEAK_SEPARATION = 3;  // points, 15 MHz

void ScanScreen::enter() {
    bool full = app::cfg.d.scanFull;
    fmin_ = full ? FREQ_MIN : SCAN_NARROW_MIN;
    fmax_ = full ? FREQ_MAX : SCAN_NARROW_MAX;
    n_ = (fmax_ - fmin_) / SCAN_STEP + 1;
    memset(rssi_, 0, sizeof(rssi_));
    idx_ = 0;
    passes_ = 0;
    peakCount_ = 0;
    cursor_ = 0;
    app::rx.active().setFrequency(freqAt(0));
}

void ScanScreen::tick() {
    Rx5808 &r = app::rx.active();
    if (!r.settled()) return;
    rssi_[idx_] = r.readRssi();
    if (++idx_ >= n_) {
        idx_ = 0;
        passes_++;
        findPeaks();
    }
    r.setFrequency(freqAt(idx_));
}

void ScanScreen::findPeaks() {
    // noise floor = median of the sweep
    uint8_t sorted[MAX_POINTS];
    memcpy(sorted, rssi_, n_);
    for (int i = 1; i < n_; i++) {
        uint8_t v = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j] > v) sorted[j + 1] = sorted[j], j--;
        sorted[j + 1] = v;
    }
    int level = sorted[n_ / 2] + app::cfg.d.threshold;

    // strongest points first, skipping neighbours of already accepted peaks
    peakCount_ = 0;
    bool used[MAX_POINTS] = {false};
    while (peakCount_ < MAX_PEAKS) {
        int best = -1;
        for (int i = 0; i < n_; i++)
            if (!used[i] && (best < 0 || rssi_[i] > rssi_[best])) best = i;
        if (best < 0 || rssi_[best] < level) break;
        for (int i = max(0, best - PEAK_SEPARATION); i <= min(n_ - 1, best + PEAK_SEPARATION); i++) used[i] = true;

        uint16_t f = freqAt(best);
        ChannelRef ref = findChannel(f, SCAN_STEP + 1);
        peaks_[peakCount_++] = {ref.valid() ? BAND_FREQS[ref.band][ref.ch] : f, rssi_[best], (uint8_t)best};
    }
    if (cursor_ >= peakCount_) cursor_ = 0;
}

void ScanScreen::handle(const KeyEvent &ev) {
    if (peakCount_ == 0) return;
    switch (ev.key) {
    case Key::Up:   cursor_ = (cursor_ + peakCount_ - 1) % peakCount_; break;
    case Key::Down: cursor_ = (cursor_ + 1) % peakCount_; break;
    case Key::Ok:
        app::tune(peaks_[cursor_].freq);
        app::switchTo(Mode::Monitor);
        break;
    default: break;
    }
}

void ScanScreen::draw(LGFX_Sprite &c) {
    sourceBadge(c);
    centerText(c, "SCAN", 44, &fonts::Font2, COL_DIM);

    // spectrum
    c.drawFastHLine(GX0, GY1 + 1, GW, COL_BG_ARC);
    for (int i = 0; i < n_; i++) {
        int x0 = GX0 + i * GW / n_;
        int x1 = GX0 + (i + 1) * GW / n_;
        int w = max(1, x1 - x0 - (n_ < 100 ? 1 : 0));
        int h = rssi_[i] * GH / 100;
        if (h > 0) c.fillRect(x0, GY1 - h, w, h, rssiColor(rssi_[i]));
    }
    int sweepX = GX0 + idx_ * GW / n_;
    c.drawFastVLine(sweepX, GY1 - GH, GH, COL_DIM);

    char buf[24];
    c.setFont(&fonts::Font0);
    c.setTextColor(COL_DIM);
    c.setTextDatum(textdatum_t::top_left);
    snprintf(buf, sizeof(buf), "%u", fmin_);
    c.drawString(buf, GX0, GY1 + 5);
    c.setTextDatum(textdatum_t::top_right);
    snprintf(buf, sizeof(buf), "%u", fmax_);
    c.drawString(buf, GX0 + GW, GY1 + 5);

    if (peakCount_ == 0) {
        centerText(c, passes_ == 0 ? "scanning..." : "no signal", 186, &fonts::Font4, COL_DIM);
        return;
    }

    // selected peak marker and info
    const Peak &p = peaks_[cursor_];
    int px = GX0 + p.index * GW / n_ + GW / n_ / 2;
    int py = GY1 - p.rssi * GH / 100 - 4;
    c.fillTriangle(px, py, px - 5, py - 8, px + 5, py - 8, TFT_WHITE);

    char name[3];
    channelName(findChannel(p.freq), name);
    snprintf(buf, sizeof(buf), "%s %u  %u%%", name, p.freq, p.rssi);
    centerText(c, buf, 184, &fonts::Font4, rssiColor(p.rssi));

    snprintf(buf, sizeof(buf), "%u/%u  OK: tune", cursor_ + 1, peakCount_);
    hint(c, buf);
}
