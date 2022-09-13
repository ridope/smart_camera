
#include "amp_send.h"

shared_data_t data __attribute__ ((section ("shared_ram")));

void amp_send_init(void)
{
    data.flag = 0;
}

void amp_send_class(uint8_t p_class)
{
    while(data.flag == 1);

    if (data.flag == 0)
    {
        data.predicted_class = p_class;
        data.flag = 1;
    }   
}
