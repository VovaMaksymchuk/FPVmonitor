#pragma once
#include "display.h"

namespace ui {

constexpr int CX = 120;
constexpr int CY = 120;

constexpr uint16_t COL_BG_ARC = lgfx::color565(40, 40, 40);
constexpr uint16_t COL_DIM = lgfx::color565(110, 110, 110);
constexpr uint16_t COL_SEL = lgfx::color565(20, 70, 160);
constexpr uint16_t COL_EDIT = lgfx::color565(255, 200, 0);
constexpr uint16_t COL_INT = lgfx::color565(0, 190, 220);
constexpr uint16_t COL_EXT = lgfx::color565(255, 140, 0);
constexpr uint16_t COL_ALARM = lgfx::color565(170, 0, 0);

uint16_t rssiColor(uint8_t pct);

void text(LGFX_Sprite &c, const char *s, int x, int y, const lgfx::IFont *font, uint16_t color);
inline void centerText(LGFX_Sprite &c, const char *s, int y, const lgfx::IFont *font, uint16_t color) {
    text(c, s, CX, y, font, color);
}

// INT / EXT badge showing which receiver the DPDT switch selected
void sourceBadge(LGFX_Sprite &c, int y = 22);

// RSSI gauge along the screen edge, open at the bottom
void rssiArc(LGFX_Sprite &c, uint8_t pct);

// Small grey line in the bottom gap
void hint(LGFX_Sprite &c, const char *s);

// Frequency scale along the edge (f0..f1 over 270 deg), open at the bottom
// like the RSSI gauge. Used by Scan and Detector.
struct FreqRing {
    static constexpr float A0 = 135, SPAN = 270;
    static constexpr int R0 = 104, R1 = 119;

    uint16_t f0 = 0, f1 = 0;

    float angle(uint16_t f) const;
    void drawScale(LGFX_Sprite &c) const;   // base line, ticks every 100 MHz, labels
    void drawBar(LGFX_Sprite &c, uint16_t f, uint8_t level, float halfDeg, uint16_t col) const;
    void drawSelected(LGFX_Sprite &c, uint16_t f) const;   // video channel mark
    void drawCursor(LGFX_Sprite &c, uint16_t f) const;     // current scan position
};

// Result list on Scan / Detector: row i of "R3 5732  85%"
constexpr int LIST_ROWS = 5;
void channelRow(LGFX_Sprite &c, int row, uint16_t freq, uint8_t level, bool selected, bool alarm);

// Bottom of Scan / Detector: INT/EXT badge and the video receiver channel
void videoFooter(LGFX_Sprite &c, uint16_t freq);

}  // namespace ui
