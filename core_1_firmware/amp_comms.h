/** @file amp_comms.h
 *  @brief Header file that controls the AMP cores communication
 *
 *  @author Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
 */

#ifndef AMP_COMMS_H
#define AMP_COMMS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define KEY_SIZE_BITS    128
#define KEY_SIZE_BYTES   16
#define NONCE_SIZE  	 13
#define MAC_LEN 	     16

#define CLASS_SIZE        1

#define AMP_COMMS_DATA_SIZE 128

typedef enum AMP_CMDS
{
    AMP_NULL,
    AMP_GET_IMG,
    AMP_GET_AES_PRIV_DATA,
    AMP_GET_PREDICTION,
    AMP_SEND_IMG,
    AMP_SEND_AES_PRIV_DATA,
    AMP_SEND_PREDICTION,
    
} amp_cmds_t;

typedef volatile struct AMP_COMMS_TX
{  
    amp_cmds_t cmd;
    uint8_t data[AMP_COMMS_DATA_SIZE];
    uint8_t flag_read;
} amp_comms_tx_t;

typedef volatile struct AMP_COMMS_RX
{
    amp_cmds_t cmd;
    uint8_t data[AMP_COMMS_DATA_SIZE];
    uint8_t flag_received;
} amp_comms_rx_t;

typedef struct PRIVATE_AES_DATA
{
    uint8_t nonce[NONCE_SIZE];
    uint8_t key[KEY_SIZE_BYTES];
    uint8_t ciphertext[CLASS_SIZE+MAC_LEN];
} private_aes_data_t;

int amp_comms_init(amp_comms_tx_t *tx_ctrl, amp_comms_rx_t *rx_ctrl);
int amp_comms_send(amp_comms_tx_t *tx_ctrl, amp_cmds_t cmd, uint8_t *data, size_t len);
int amp_comms_receive(amp_comms_rx_t *rx_ctrl, uint8_t *data, size_t len);
amp_cmds_t amp_comms_has_unread(amp_comms_rx_t *rx_ctrl);

#endif