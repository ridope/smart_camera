
#include "amp_send.h"

shared_data_t data __attribute__ ((section ("shared_ram_first")));

/**
 * @brief Inits the control flag structure 
 * 
 */
void amp_send_init(void)
{
    data.flag = 0;
}

/**
 * @brief Sends the predicted class, it blocks if didn't finish
 * 
 * @param p_class   The predicted class
 */
void amp_send_class(uint8_t p_class)
{
    /* Waiting for encryption to finish */
    while(data.flag == 1);

    if (data.flag == 0)
    {
        data.predicted_class = p_class;
        data.flag = 1;
    }  
}