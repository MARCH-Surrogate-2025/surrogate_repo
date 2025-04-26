#ifndef EL3068_H
#define EL3068_H

#include "ethercat.h"
#include <cstdint>
#include <cstdio>

class EL3068
{
public:
    EL3068();
    void init(uint16 slave);
    void readChannels();
    int getChannel(int ch);

private:
    uint16 slave_num;
    uint8_t* ai_data;

    static constexpr int channel_offset[3] = {2, 6, 10};
    static constexpr int OVERSAMPLE_COUNT = 8;
    static constexpr float alpha = 0.05f;

    float filtered_ch[3];
    int p_ch[3];

    int oversample_channel(int offset);
    void update_ema(float& filtered_value, int16_t new_value);
};

#endif // EL3068_H
