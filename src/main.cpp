#include <Arduino.h>
#include "app.h"
#include "config.h"
#include "ui/common.h"
#include "ui/screens.h"

namespace app {
Receivers rx;
Settings cfg;
Buzzer buzzer;
}  // namespace app

static Buttons buttons;

static MenuScreen menuScreen;
static MonitorScreen monitorScreen;
static ScanScreen scanScreen;
static DetectorScreen detectorScreen;
static SettingsScreen settingsScreen;

static Screen *const SCREENS[] = {&menuScreen, &monitorScreen, &scanScreen, &detectorScreen, &settingsScreen};
static Screen *current = nullptr;
static Mode currentMode = Mode::Menu;
static uint32_t lastFrame = 0;

namespace app {

Mode mode() { return currentMode; }

void switchTo(Mode m) {
    if (current) current->leave();
    currentMode = m;
    current = SCREENS[(int)m];
    if (m == Mode::Monitor || m == Mode::Scan || m == Mode::Detector) {
        cfg.d.mode = (uint8_t)m;
        cfg.markDirty();
    }
    current->enter();
}

void tune(uint16_t freq) {
    if (freq != cfg.d.freq) {
        cfg.d.freq = freq;
        cfg.markDirty();
    }
    rx.active().setFrequency(freq);
}

void applyCalibration() {
    for (int i = 0; i < 2; i++) rx.rx((Source)i).setCalibration(cfg.d.rssiMin[i], cfg.d.rssiMax[i]);
}

}  // namespace app

static void splash() {
    canvas.fillScreen(TFT_BLACK);
    canvas.drawCircle(ui::CX, ui::CY, 118, ui::COL_SEL);
    ui::centerText(canvas, "FPV", 100, &fonts::FreeSansBold24pt7b, TFT_WHITE);
    ui::centerText(canvas, "MONITOR", 140, &fonts::Font4, ui::COL_DIM);
    ui::sourceBadge(canvas, 180);
    displayPush();
}

void setup() {
    Serial.begin(115200);

    app::cfg.load();
    app::rx.begin();
    app::applyCalibration();
    app::buzzer.begin(PIN_BUZZER);
    app::buzzer.setEnabled(app::cfg.d.buzzer);
    buttons.begin();

    if (!displayBegin()) Serial.println("display: no memory for frame buffer");
    splash();
    app::buzzer.beep(2000, 60);
    delay(800);

    Serial.printf("FPV monitor: source %s, %u MHz\n", app::rx.sourceName(), app::cfg.d.freq);
    app::switchTo((Mode)app::cfg.d.mode);
}

void loop() {
    if (app::rx.update()) {
        Serial.printf("source -> %s\n", app::rx.sourceName());
        app::buzzer.beep(1800, 80);
        current->enter();  // retune the newly selected receiver
    }

    buttons.update();
    for (KeyEvent ev = buttons.poll(); ev.key != Key::None; ev = buttons.poll()) {
        if (ev.repeat == 0) app::buzzer.click();
        if (ev.key == Key::OkLong) {
            if (currentMode != Mode::Menu) app::switchTo(Mode::Menu);
            continue;
        }
        current->handle(ev);
    }

    current->tick();
    app::buzzer.update();
    app::cfg.update();

    uint32_t now = millis();
    if (now - lastFrame >= FRAME_MS) {
        lastFrame = now;
        canvas.fillScreen(TFT_BLACK);
        current->draw(canvas);
        displayPush();
    }
}
