#include "ui/common.h"
#include "app.h"
#include "channels.h"

namespace ui {

uint16_t rssiColor(uint8_t pct) {
    if (pct < 30) return TFT_RED;
    if (pct < 60) return TFT_YELLOW;
    return TFT_GREEN;
}

void text(LGFX_Sprite &c, const char *s, int x, int y, const lgfx::IFont *font, uint16_t color) {
    c.setFont(font);
    c.setTextDatum(textdatum_t::middle_center);
    c.setTextColor(color);
    c.drawString(s, x, y);
}

void sourceBadge(LGFX_Sprite &c, int y) {
    bool ext = app::rx.source() == Source::External;
    uint16_t col = ext ? COL_EXT : COL_INT;
    c.fillRoundRect(CX - 22, y - 9, 44, 18, 6, col);
    text(c, app::rx.sourceName(), CX, y + 1, &fonts::Font2, TFT_BLACK);
}

void rssiArc(LGFX_Sprite &c, uint8_t pct) {
    c.fillArc(CX, CY, 110, 118, 135, 405, COL_BG_ARC);
    if (pct > 0) c.fillArc(CX, CY, 110, 118, 135, 135 + 270.0f * pct / 100, rssiColor(pct));
}

void hint(LGFX_Sprite &c, const char *s) {
    centerText(c, s, 216, &fonts::Font2, COL_DIM);
}

static constexpr int LABEL_R = 90;
static constexpr int LABEL_STEP = 100;  // MHz between frequency labels

float FreqRing::angle(uint16_t f) const {
    float t = f1 > f0 ? (float)((int)constrain(f, f0, f1) - f0) / (f1 - f0) : 0.5f;
    return A0 + t * SPAN;
}

void FreqRing::drawScale(LGFX_Sprite &c) const {
    c.fillArc(CX, CY, R0 - 2, R0 - 1, A0, A0 + SPAN, COL_BG_ARC);

    char buf[8];
    c.setFont(&fonts::Font0);
    c.setTextDatum(textdatum_t::middle_center);
    c.setTextColor(COL_DIM);
    for (int f = (f0 / LABEL_STEP + 1) * LABEL_STEP; f < f1; f += LABEL_STEP) {
        float a = angle(f) * DEG_TO_RAD;
        c.drawLine(CX + cosf(a) * (R0 - 6), CY + sinf(a) * (R0 - 6),
                   CX + cosf(a) * (R0 - 2), CY + sinf(a) * (R0 - 2), COL_DIM);
        snprintf(buf, sizeof(buf), "%d", f);
        c.drawString(buf, CX + cosf(a) * LABEL_R, CY + sinf(a) * LABEL_R);
    }
    for (uint16_t f : {f0, f1}) {  // range edges
        float a = angle(f) * DEG_TO_RAD;
        snprintf(buf, sizeof(buf), "%u", f);
        c.drawString(buf, CX + cosf(a) * LABEL_R, CY + sinf(a) * LABEL_R);
    }
}

void FreqRing::drawBar(LGFX_Sprite &c, uint16_t f, uint8_t level, float halfDeg, uint16_t col) const {
    float a = angle(f);
    int len = 2 + level * (R1 - R0 - 2) / 100;
    c.fillArc(CX, CY, R0, R0 + len, a - halfDeg, a + halfDeg, col);
}

void FreqRing::drawSelected(LGFX_Sprite &c, uint16_t f) const {
    float a = angle(f);
    c.fillArc(CX, CY, R0 - 9, R0 - 3, a - 2, a + 2, COL_EDIT);
}

void FreqRing::drawCursor(LGFX_Sprite &c, uint16_t f) const {
    float a = angle(f) * DEG_TO_RAD;
    c.fillCircle(CX + cosf(a) * (R1 + 1), CY + sinf(a) * (R1 + 1), 2, TFT_WHITE);
}

void channelRow(LGFX_Sprite &c, int row, uint16_t freq, uint8_t level, bool selected, bool alarm) {
    int y = 70 + row * 22;
    if (selected) c.fillRoundRect(CX - 62, y - 10, 124, 20, 6, COL_SEL);
    char name[3], buf[20];
    channelName(findChannel(freq), name);
    snprintf(buf, sizeof(buf), "%s %u %3u%%", name, freq, level);
    uint16_t col = alarm ? (selected ? TFT_YELLOW : TFT_RED) : selected ? TFT_WHITE : rssiColor(level);
    centerText(c, buf, y + 1, &fonts::FreeSansBold9pt7b, col);
}

void videoFooter(LGFX_Sprite &c, uint16_t freq) {
    sourceBadge(c, 200);
    char name[3], buf[20];
    channelName(findChannel(freq), name);
    snprintf(buf, sizeof(buf), "video %s %u", name, freq);
    centerText(c, buf, 220, &fonts::Font0, COL_EDIT);
}

}  // namespace ui
