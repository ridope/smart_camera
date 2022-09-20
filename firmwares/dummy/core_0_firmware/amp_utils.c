#include "amp_utils.h"

/**
 * @brief Function to start and configure a 1ms Timer0
 * 
 */
void amp_millis_init(void)
{
    timer0_en_write(0);
    timer0_load_write(0);
    timer0_reload_write(0xFFFFFFFF);
    timer0_en_write(1);
    timer0_update_value_write(1);
}

/**
 * @brief Returns the actual value of counter of the timer
 * 
 * @return uint32_t  The counter value of the timer
 */
uint32_t amp_millis(void)
{
    timer0_update_value_write(1);
    return timer0_value_read();
}
