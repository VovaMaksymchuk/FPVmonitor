#include "ui/screens.h"
#include "ui/common.h"

using namespace ui;

static const struct {
    const char *name;
    Mode mode;
} ITEMS[] = {
    {"Monitor", Mode::Monitor},
    {"Scan", Mode::Scan},
    {"Detector", Mode::Detector},
    {"Settings", Mode::Settings},
};
static constexpr int ITEM_COUNT = sizeof(ITEMS) / sizeof(ITEMS[0]);

void MenuScreen::enter() {
    app::tune(app::cfg.d.freq);
}

void MenuScreen::handle(const KeyEvent &ev) {
    switch (ev.key) {
    case Key::Up:   cursor_ = (cursor_ + ITEM_COUNT - 1) % ITEM_COUNT; break;
    case Key::Down: cursor_ = (cursor_ + 1) % ITEM_COUNT; break;
    case Key::Ok:   app::switchTo(ITEMS[cursor_].mode); break;
    default: break;
    }
}

void MenuScreen::draw(LGFX_Sprite &c) {
    sourceBadge(c);
    centerText(c, "FPV MONITOR", 46, &fonts::Font2, COL_DIM);

    for (int i = 0; i < ITEM_COUNT; i++) {
        int y = 78 + i * 34;
        if (i == cursor_) c.fillRoundRect(CX - 75, y - 15, 150, 30, 10, COL_SEL);
        centerText(c, ITEMS[i].name, y + 1, &fonts::Font4, TFT_WHITE);
    }

    char name[3], buf[16];
    channelName(findChannel(app::cfg.d.freq), name);
    snprintf(buf, sizeof(buf), "%s  %u", name, app::cfg.d.freq);
    hint(c, buf);
}
