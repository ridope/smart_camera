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

#include <mbedtls/gcm.h>
#include <mbedtls/cipher.h>
#include <tinycrypt/ctr_prng.h>


__attribute__((section(".ram_code"))) int amp_aes_init(private_firev_data_t *priv_data);
__attribute__((section(".ram_code"))) int amp_aes_update_nonce(private_firev_data_t *priv_data);
__attribute__((section(".ram_code"))) int amp_aes_encrypts(shared_data_t *data_ctrl, private_firev_data_t *priv_d);
__attribute__((section(".ram_code"))) uint8_t get_hex_rep(char *str_input, uint8_t in_size, uint8_t *hex_out);

#endif