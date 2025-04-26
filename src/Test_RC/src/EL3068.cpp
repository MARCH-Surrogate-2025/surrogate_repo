#include "EL3068.h"
#include <cstring>

extern char IOmap[];

EL3068::EL3068()
{
    slave_num = 0;
    ai_data = nullptr;
    memset(filtered_ch, 0, sizeof(filtered_ch));
    memset(p_ch, 0, sizeof(p_ch));
}

void EL3068::init(uint16 slave)
{
    slave_num = slave;
    ai_data = reinterpret_cast<uint8_t*>(IOmap);  // IOmap을 직접 가리킴
    //ai_data = ec_slave[2].inputs;

    if (ai_data == nullptr)
    {
        printf("ERROR: Slave %d ai_data pointer is NULL!\n", slave_num);
    }
    else
    {
        printf("Slave %d ai_data pointer is valid: %p\n", slave_num, ai_data);
    }
}


int EL3068::oversample_channel(int offset)
{
    int32_t sum = 0;
    for (int i = 0; i < OVERSAMPLE_COUNT; i++)
    {
        int16_t raw = *(int16_t*)(ai_data + offset);
        sum += raw;
    }
    return static_cast<int>(sum / OVERSAMPLE_COUNT);
}

void EL3068::update_ema(float& filtered_value, int16_t new_value)
{
    filtered_value = alpha * new_value + (1.0f - alpha) * filtered_value;
}

void EL3068::readChannels()
{
    if (ai_data == nullptr)
        return;

    printf("ai_data raw dump: ");
    for (int i = 0; i < 12; i++)
    {
        printf("%02X ", ai_data[i]);
    }
    printf("\n");


    for (int i = 0; i < 3; i++)
    {
        int16_t raw = *(int16_t*)(ai_data + channel_offset[i]);
        update_ema(filtered_ch[i], raw);
        p_ch[i] = static_cast<int>(filtered_ch[i]);
    }
}


int EL3068::getChannel(int ch)
{
    if (ch < 1 || ch > 3) return 0;
    return p_ch[ch - 1];
}
