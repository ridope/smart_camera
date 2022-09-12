
#include "amp_send.h"

shared_data_t data __attribute__ ((section ("shared_ram")));

void amp_send_init(void)
{
    data.flag = 0;
}

void amp_send_class(uint8_t p_class)
{
    printf("Hello I am femtorv; the class is %d \n", p_class);
    printf("Hello I am femtorv; the data flag is %d \n", data.flag);
    printf("Hello I am femtorv; the data addr is %p \n", &data);

    if (data.flag == 0)
    {
        data.predicted_class = p_class;
        data.flag = 1;
    }

}
