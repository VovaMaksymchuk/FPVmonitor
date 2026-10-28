#include "ui/screens.h"
#include "ui/common.h"

using namespace ui;

static constexpr int PEAK_SEPARATION = 3;  // points, 15 MHz

void ScanScreen::enter() {
    selFreq_ = app::cfg.d.freq;
    app::rx.active().setFrequency(selFreq_);  // video receiver keeps its channel
    start();
}

void ScanScreen::start() {
    bool full = app::cfg.d.scanFull;
    ring_.f0 = full ? FREQ_MIN : SCAN_NARROW_MIN;
    n_ = ((full ? FREQ_MAX : SCAN_NARROW_MAX) - ring_.f0) / SCAN_STEP + 1;
    ring_.f1 = freqAt(n_ - 1);
    memset(rssi_, 0, sizeof(rssi_));
    idx_ = 0;
    done_ = false;
    peakCount_ = 0;
    startedAt_ = millis();
    app::rx.scanner().setFrequency(freqAt(0));
}

void ScanScreen::tick() {
    if (done_) return;
    Rx5808 &r = app::rx.scanner();
    if (!r.settled()) return;
    rssi_[idx_] = r.readRssi();
    if (++idx_ < n_) {
        r.setFrequency(freqAt(idx_));
        return;
    }
    done_ = true;
    scanMs_ = millis() - startedAt_;
    findPeaks();
    app::buzzer.beep(peakCount_ ? 3000 : 1500, 80);
#if DEBUG_LOG
    Serial.printf("[SCAN] %u points in %u ms, %u signals\n", n_, scanMs_, peakCount_);
    for (int i = 0; i < peakCount_; i++) Serial.printf("  %u MHz  %u%%\n", peaks_[i].freq, peaks_[i].rssi);
#endif
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
    level_ = min(100, sorted[n_ / 2] + app::cfg.d.threshold);

    // strongest points first, skipping neighbours of already accepted peaks
    peakCount_ = 0;
    bool used[MAX_POINTS] = {false};
    while (peakCount_ < LIST_ROWS) {
        int best = -1;
        for (int i = 0; i < n_; i++)
            if (!used[i] && (best < 0 || rssi_[i] > rssi_[best])) best = i;
        if (best < 0 || rssi_[best] < level_) break;
        for (int i = max(0, best - PEAK_SEPARATION); i <= min(n_ - 1, best + PEAK_SEPARATION); i++) used[i] = true;

        uint16_t f = freqAt(best);
        ChannelRef ref = findChannel(f, SCAN_STEP + 1);
        peaks_[peakCount_++] = {ref.valid() ? BAND_FREQS[ref.band][ref.ch] : f, rssi_[best]};
    }
}

void ScanScreen::handle(const KeyEvent &ev) {
    if (ev.key == Key::Ok) {
        if (done_) start();
        return;
    }
    if (!done_ || peakCount_ == 0 || (ev.key != Key::Up && ev.key != Key::Down)) return;

    int pos = -1;
    for (int i = 0; i < peakCount_; i++)
        if (peaks_[i].freq == selFreq_) pos = i;
    if (pos < 0) pos = 0;
    else pos = (pos + (ev.key == Key::Up ? peakCount_ - 1 : 1)) % peakCount_;
    selFreq_ = peaks_[pos].freq;
    app::tune(selFreq_);
}

void ScanScreen::draw(LGFX_Sprite &c) {
    // spectrum on the ring
    ring_.drawScale(c);
    float half = FreqRing::SPAN / n_ * 0.45f;
    int shown = done_ ? n_ : idx_;
    for (int i = 0; i < shown; i++) {
        uint16_t col = !done_ || rssi_[i] >= level_ ? rssiColor(rssi_[i]) : COL_DIM;
        ring_.drawBar(c, freqAt(i), rssi_[i], half, col);
    }
    ring_.drawSelected(c, selFreq_);

    char buf[24];
    if (!done_) {
        ring_.drawCursor(c, freqAt(idx_));
        centerText(c, "SCAN", 50, &fonts::Font2, COL_DIM);
        snprintf(buf, sizeof(buf), "%u%%", idx_ * 100 / n_);
        centerText(c, buf, 104, &fonts::Font4, TFT_WHITE);
        snprintf(buf, sizeof(buf), "%u MHz", freqAt(idx_));
        centerText(c, buf, 132, &fonts::Font2, COL_DIM);
        videoFooter(c, selFreq_);
        return;
    }

    // result
    snprintf(buf, sizeof(buf), "SCAN %u.%us", scanMs_ / 1000, scanMs_ % 1000 / 100);
    centerText(c, buf, 50, &fonts::Font2, COL_DIM);
    if (peakCount_ == 0) centerText(c, "no signals", 110, &fonts::Font4, COL_DIM);
    for (int i = 0; i < peakCount_; i++)
        channelRow(c, i, peaks_[i].freq, peaks_[i].rssi, peaks_[i].freq == selFreq_, false);

    snprintf(buf, sizeof(buf), "%u sig  OK:rescan", peakCount_);
    centerText(c, buf, 182, &fonts::Font0, COL_DIM);
    videoFooter(c, selFreq_);
}
