
#include "send.h"

DATA data __attribute__ ((section ("joseph")));

void amp_send_init(void)
{
    data.flag = 0;
}

void amp_send_class(uint8_t class)
{
    printf("Hello I am femtorv; the class is %d \n", class);
    printf("Hello I am femtorv; the data flag is %d \n", data.flag);
    printf("Hello I am femtorv; the data addr is %p \n", &data);

    if (data.flag == 0)
    {
        data.predicted_class = class;
        data.flag = 1;
    }

}
