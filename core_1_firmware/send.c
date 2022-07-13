
#include "send.h"

DATA data __attribute__ ((section ("joseph")));

void amp_send_class(uint8_t class)
{
    printf("Hello I am fireV; the class is %d \n", class);

    if (data.flag == 0)
    {
        data.predicted_class = class;
        data.flag = 1;
    }

}
