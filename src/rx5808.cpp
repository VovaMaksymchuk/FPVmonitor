#include "rx5808.h"
#include "config.h"

Rx5808::Rx5808(int dataPin, int clkPin, int lePin, int rssiPin)
    : data_(dataPin), clk_(clkPin), le_(lePin), rssi_(rssiPin),
      mvMin_(RSSI_MV_MIN_DEF), mvMax_(RSSI_MV_MAX_DEF) {}

void Rx5808::begin() {
    pinMode(data_, OUTPUT);
    pinMode(clk_, OUTPUT);
    pinMode(le_, OUTPUT);
    digitalWrite(clk_, LOW);
    digitalWrite(le_, HIGH);
    pinMode(rssi_, INPUT);
    analogSetPinAttenuation(rssi_, ADC_11db);
}

void Rx5808::sendBit(bool bit) {
    digitalWrite(data_, bit ? HIGH : LOW);
    delayMicroseconds(1);
    digitalWrite(clk_, HIGH);
    delayMicroseconds(1);
    digitalWrite(clk_, LOW);
    delayMicroseconds(1);
}

// 25-bit frame, LSB first: 4 bits address, 1 bit R/W (1 = write), 20 bits data.
void Rx5808::sendRegister(uint8_t addr, uint32_t data) {
    digitalWrite(le_, LOW);
    delayMicroseconds(1);
    for (int i = 0; i < 4; i++) sendBit(addr & (1 << i));
    sendBit(true);
    for (int i = 0; i < 20; i++) sendBit(data & (1UL << i));
    digitalWrite(le_, HIGH);
    delayMicroseconds(1);
}

// Synthesizer register B: f_LO = f_RF - 479 MHz = 2 * (N * 32 + A)
void Rx5808::setFrequency(uint16_t mhz) {
    mhz = constrain(mhz, FREQ_MIN, FREQ_MAX);
    uint32_t half = (mhz - 479) / 2;
    uint32_t n = half / 32;
    uint32_t a = half % 32;
    sendRegister(0x01, (n << 7) | a);
    freq_ = mhz;
    tunedAt_ = millis();
}

bool Rx5808::settled() const {
    return millis() - tunedAt_ >= RSSI_SETTLE_MS;
}

uint16_t Rx5808::readRssiMv() {
    uint32_t sum = 0;
    for (int i = 0; i < RSSI_SAMPLES; i++) sum += analogReadMilliVolts(rssi_);
    return sum / RSSI_SAMPLES;
}

uint8_t Rx5808::mvToPercent(uint16_t mv) const {
    if (mv <= mvMin_) return 0;
    if (mv >= mvMax_) return 100;
    return (uint32_t)(mv - mvMin_) * 100 / (mvMax_ - mvMin_);
}

uint8_t Rx5808::readRssi() {
    return mvToPercent(readRssiMv());
}

void Rx5808::setCalibration(uint16_t mvMin, uint16_t mvMax) {
    if (mvMax <= mvMin) return;
    mvMin_ = mvMin;
    mvMax_ = mvMax;
}
