/** @file amp_aes.h
 *  @brief Header file performing the AES-128 Encryption
 *
 *  @author Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
 */

#ifndef AES_H
#define AES_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data.h"

#include <tinycrypt/ctr_prng.h>
#include <tinycrypt/aes.h>
#include <tinycrypt/ccm_mode.h>
#include <tinycrypt/constants.h>

int amp_aes_init(private_firev_data_t *priv_data);
int amp_aes_update_nonce(shared_data_t *data_ctrl, private_firev_data_t *priv_data);
int amp_aes_encrypts(shared_data_t *data_ctrl, private_firev_data_t *priv_d);
uint8_t get_hex_rep(char *str_input, uint8_t in_size, uint8_t *hex_out);

#endif