#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pin map for ESP32-S3 Super Mini (check against your board before soldering)
// ---------------------------------------------------------------------------

// RSSI inputs (must be ADC1 pins)
#define PIN_RSSI_INT    1
#define PIN_RSSI_EXT    2
#define PIN_RSSI_SCAN   17          // dedicated scanner module (ADC2 is fine, no Wi-Fi)

// Buttons, active low (to GND, internal pull-up)
#define PIN_BTN_UP      3
#define PIN_BTN_OK      4
#define PIN_BTN_DOWN    5

// RX5808 SPI bus (bit-banged). DATA/CLK are shared, LE is per module.
// Third module (SCAN) has no video: it runs Scan/Detector while INT/EXT keep the picture.
#define PIN_RX_DATA     6
#define PIN_RX_CLK      7
#define PIN_RX_LE_INT   8
#define PIN_RX_LE_EXT   13
#define PIN_RX_LE_SCAN  16

// GC9A01 (RST and BLK are driven by GPIO; BLK is PWM-dimmed)
#define PIN_TFT_DC      9
#define PIN_TFT_CS      10
#define PIN_TFT_MOSI    11
#define PIN_TFT_SCK     12
#define PIN_TFT_RST     14
#define PIN_TFT_BL      15
#define TFT_BL_DEFAULT  255         // 0..255
#define TFT_SPI_HZ      40000000

// Buzzer through an NPN transistor
#define PIN_BUZZER      43

// Second pole of the DPDT video switch: EXT position shorts it to GND
#define PIN_SRC_SENSE   44

// ---------------------------------------------------------------------------
// Radio
// ---------------------------------------------------------------------------
#define FREQ_MIN        5300
#define FREQ_MAX        5999
#define FREQ_DEFAULT    5658        // R1

#define SCAN_NARROW_MIN 5645
#define SCAN_NARROW_MAX 5945
#define SCAN_STEP       5

#define RSSI_SETTLE_MS  30          // RSSI is valid this long after retune
#define RSSI_SAMPLES    8           // ADC averaging
#define RSSI_MV_MIN_DEF 500         // default calibration, mV
#define RSSI_MV_MAX_DEF 1200

// ---------------------------------------------------------------------------
// Detector
// ---------------------------------------------------------------------------
#define DET_THRESHOLD_DEF   25      // % above noise floor
#define DET_HYSTERESIS      8       // % below threshold before a channel is inactive again

// ---------------------------------------------------------------------------
// UI timing
// ---------------------------------------------------------------------------
#define FRAME_MS            33
#define BTN_DEBOUNCE_MS     25
#define BTN_LONG_MS         700
#define BTN_REPEAT_DELAY_MS 400
#define BTN_REPEAT_MS       120
#define BTN_REPEAT_FAST_MS  40
#define SETTINGS_SAVE_DELAY 2000

#define DEBUG_LOG           1
