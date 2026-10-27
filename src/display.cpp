#include "display.h"
#include "config.h"

LGFX tft;
LGFX_Sprite canvas(&tft);

LGFX::LGFX() {
    {
        auto cfg = bus_.config();
        cfg.spi_host = SPI2_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = TFT_SPI_HZ;
        cfg.freq_read = 16000000;
        cfg.pin_sclk = PIN_TFT_SCK;
        cfg.pin_mosi = PIN_TFT_MOSI;
        cfg.pin_miso = -1;
        cfg.pin_dc = PIN_TFT_DC;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        bus_.config(cfg);
        panel_.setBus(&bus_);
    }
    {
        auto cfg = panel_.config();
        cfg.pin_cs = PIN_TFT_CS;
        cfg.pin_rst = PIN_TFT_RST;
        cfg.pin_busy = -1;
        cfg.panel_width = 240;
        cfg.panel_height = 240;
        cfg.memory_width = 240;
        cfg.memory_height = 240;
        cfg.invert = true;
        cfg.rgb_order = false;
        cfg.readable = false;
        cfg.bus_shared = false;
        panel_.config(cfg);
    }
    {
        auto cfg = light_.config();
        cfg.pin_bl = PIN_TFT_BL;
        cfg.invert = false;
        cfg.freq = 12000;
        cfg.pwm_channel = 7;
        light_.config(cfg);
        panel_.setLight(&light_);
    }
    setPanel(&panel_);
}

bool displayBegin() {
    tft.init();
    tft.setRotation(0);
    tft.setBrightness(TFT_BL_DEFAULT);
    tft.fillScreen(TFT_BLACK);

    canvas.setColorDepth(16);
    if (!canvas.createSprite(240, 240)) {
        // not enough RAM for 16-bit: fall back to 8-bit
        canvas.setColorDepth(8);
        if (!canvas.createSprite(240, 240)) return false;
    }
    return true;
}

void displayPush() {
    canvas.pushSprite(0, 0);
}
