#pragma once
#include <Arduino.h>

// RX5808 (and compatible modules) in SPI mode.
// DATA/CLK may be shared between several modules, LE selects the module.
class Rx5808 {
public:
    Rx5808(int dataPin, int clkPin, int lePin, int rssiPin);

    void begin();
    void setFrequency(uint16_t mhz);
    uint16_t frequency() const { return freq_; }

    // RSSI is only meaningful RSSI_SETTLE_MS after the last retune.
    bool settled() const;
    uint16_t readRssiMv();          // averaged raw value
    uint8_t readRssi();             // 0..100 using calibration

    void setCalibration(uint16_t mvMin, uint16_t mvMax);
    uint8_t mvToPercent(uint16_t mv) const;

private:
    void sendRegister(uint8_t addr, uint32_t data);
    void sendBit(bool bit);

    int data_, clk_, le_, rssi_;
    uint16_t freq_ = 0;
    uint32_t tunedAt_ = 0;
    uint16_t mvMin_, mvMax_;
};
