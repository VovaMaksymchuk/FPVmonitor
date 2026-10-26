#pragma once
#include <stdint.h>

constexpr int BAND_COUNT = 6;
constexpr int CH_PER_BAND = 8;
constexpr int CHANNEL_COUNT = BAND_COUNT * CH_PER_BAND;

extern const char BAND_NAMES[BAND_COUNT];
extern const uint16_t BAND_FREQS[BAND_COUNT][CH_PER_BAND];

struct ChannelRef {
    int8_t band = -1;  // -1 when the frequency is not in any table
    int8_t ch = -1;
    bool valid() const { return band >= 0; }
};

// Closest table channel within +-tolerance MHz.
ChannelRef findChannel(uint16_t freq, uint8_t tolerance = 0);

// "R3", or "--" for a custom frequency. buf must hold 3 chars.
void channelName(ChannelRef ref, char *buf);
