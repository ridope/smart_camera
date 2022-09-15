
#include "amp_comms.h"


/**
 * @brief Inits the control communication structure s
 * 
 */

/**
 * @brief  Inits the amp communication control structure
 * 
 * @param tx_ctrl   Pointer to the structure that controls the transmited data
 * @param rx_ctrl   Pointer to the structure that controls the received data
 * @return int     Returns 0 if success, < 0 if an error occurred
 */
int amp_comms_init(amp_comms_tx_t *tx_ctrl, amp_comms_rx_t *rx_ctrl)
{
    // Checking input parameters
    if(tx_ctrl == NULL)
    {
        return -1;
    }

    if(rx_ctrl == NULL)
    {
        return -2;
    }

    memset((void *)tx_ctrl, 0, sizeof(*tx_ctrl));
    memset((void *)rx_ctrl, 0, sizeof(*rx_ctrl));

    tx_ctrl->flag_read = 1;
    rx_ctrl->flag_received = 1;
    
    return 0;
}

/**
 * @brief Sends a command with an associated data
 * 
 * @param tx_ctrl   Pointer to the structure that controls the transmited data
 * @param data      The associated data
 * @param len       Size of the associated data to be sent
 * @return int     Returns 0 if success, < 0 if an error occurred
 */
int amp_comms_send(amp_comms_tx_t *tx_ctrl, amp_cmds_t cmd, uint8_t *data, size_t len)
{
    // Checking input parameters
    if(tx_ctrl == NULL)
    {
        return -1;
    }

    if(data == NULL)
    {
        return -2;
    }

    size_t counter = 0;

    do {
          /* Waits for the last message to be read */
        while(!tx_ctrl->flag_read);

        int size_to_send;

        if(len > AMP_COMMS_DATA_SIZE)
        {
            size_to_send = AMP_COMMS_DATA_SIZE;
        }
        else
        {
            size_to_send = len;
        }

        tx_ctrl->cmd = cmd;
        memcpy((void *)&tx_ctrl->data, (void *) &data[counter], size_to_send);
        tx_ctrl->flag_read = 0;

        counter += size_to_send;

    }while(counter < len);

    return 0;
}

/**
 * @brief Receives a command with an associated data
 * 
 * @param rx_ctrl  Pointer to the structure that controls the received data
 * @param data     The associated data
 * @param len      Size of the associated data to be received
 * @return int     Returns 0 if success, < 0 if an error occurred
 */
int amp_comms_receive(amp_comms_rx_t *rx_ctrl, uint8_t *data, size_t len)
{
    // Checking input parameters
    if(rx_ctrl == NULL)
    {
        return -1;
    }

    if(data == NULL && len > 0)
    {
        return -2;
    }

    if(len == 0)
    {
        rx_ctrl->flag_received = 1;
    }

    size_t counter = 0;

    do {
          /* Waits for a new message */
        while(rx_ctrl->flag_received);

        int size_to_receive;

        if(len > AMP_COMMS_DATA_SIZE)
        {
            size_to_receive = AMP_COMMS_DATA_SIZE;
        }
        else
        {
            size_to_receive = len;
        }

        memcpy((void *) &data[counter], (void *)&rx_ctrl->data, size_to_receive);
        rx_ctrl->flag_received = 1;

        counter += size_to_receive;

    }while(counter < len);

    return 0;
}

/**
 * @brief Checks if there is an unread message
 * 
 * @param rx_ctrl       Pointer to the structure that controls the received data
 * @return amp_cmds_t   Returns AMP_NULL if there is no message, or returns the command in the message
 */
amp_cmds_t amp_comms_has_unread(amp_comms_rx_t *rx_ctrl)
{
    if(rx_ctrl == NULL)
    {
        return AMP_NULL;
    }

    if(!rx_ctrl->flag_received)
    {
        return rx_ctrl->cmd;
    }

    return AMP_NULL;
}