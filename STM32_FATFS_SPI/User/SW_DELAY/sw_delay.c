#include "sw_delay.h"


// 66 NOP
void sw_delay_us(uint32_t nTimer)
{
    uint32_t i=0;

    for(i = 0; i < nTimer; i++)
    {
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();

        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();

        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();

        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();

        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();

    }
}


void sw_delay_ms(uint32_t nTimer)
{
    uint32_t i = 1000 * nTimer;
    sw_delay_us(i);
}


