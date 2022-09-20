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

#include "amp_comms.h"
#include <tinycrypt/ctr_prng.h>
#include <tinycrypt/aes.h>
#include <tinycrypt/ccm_mode.h>
#include <tinycrypt/constants.h>

int amp_aes_init(private_aes_data_t *priv_data);
int amp_aes_update_nonce(private_aes_data_t *priv_data);
int amp_aes_encrypts(uint8_t *class, private_aes_data_t *priv_d);

#endif