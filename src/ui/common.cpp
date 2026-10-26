#include "ui/common.h"
#include "app.h"

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

}  // namespace ui
