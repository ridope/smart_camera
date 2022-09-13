
#include "amp_send.h"

shared_data_t data __attribute__ ((section ("shared_ram")));

/**
 * @brief Inits the control flag structure 
 * 
 */
void amp_send_init(void)
{
    data.flag = 0;
}

/**
 * @brief Sends the predicted class and waits for the encryption to finish
 * 
 * @param p_class   The predicted class
 */
void amp_send_class(uint8_t p_class)
{
    

    if (data.flag == 0)
    {
        data.predicted_class = p_class;
        data.flag = 1;
    }  

    while(data.flag == 1);
}
