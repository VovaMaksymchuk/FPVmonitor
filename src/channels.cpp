#include "channels.h"
#include <stdlib.h>

const char BAND_NAMES[BAND_COUNT] = {'A', 'B', 'E', 'F', 'R', 'L'};

const uint16_t BAND_FREQS[BAND_COUNT][CH_PER_BAND] = {
    {5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725},  // A (Boscam A)
    {5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866},  // B (Boscam B)
    {5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945},  // E (Boscam E)
    {5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880},  // F (Fatshark / IRC)
    {5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917},  // R (Raceband)
    {5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621},  // L (Lowband)
};

ChannelRef findChannel(uint16_t freq, uint8_t tolerance) {
    ChannelRef best;
    int bestDiff = tolerance + 1;
    for (int b = 0; b < BAND_COUNT; b++) {
        for (int c = 0; c < CH_PER_BAND; c++) {
            int diff = abs((int)BAND_FREQS[b][c] - (int)freq);
            if (diff < bestDiff) {
                bestDiff = diff;
                best.band = b;
                best.ch = c;
            }
        }
    }
    return best;
}

void channelName(ChannelRef ref, char *buf) {
    if (!ref.valid()) {
        buf[0] = '-';
        buf[1] = '-';
    } else {
        buf[0] = BAND_NAMES[ref.band];
        buf[1] = '1' + ref.ch;
    }
    buf[2] = 0;
}
