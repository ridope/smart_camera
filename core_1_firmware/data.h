
/** @file amp_data.h
 *  @brief Header file defing the data being transmited in the amp
 *
 *  @author Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
 */

#ifndef DATA_H
#define DATA_H


#include <stdlib.h>
#include <stdio.h>

#define KEY_SIZE_BITS    128
#define KEY_SIZE_BYTES   16
#define NONCE_SIZE  	 13
#define MAC_LEN 	     16

#define CLASS_SIZE        1

typedef volatile struct SHARED_DATA {
    uint8_t predicted_class;
    int flag;
} shared_data_t;

typedef struct PRIVATE_FIREV_DATA {
    uint8_t nonce[NONCE_SIZE];
    uint8_t key[KEY_SIZE_BYTES];

    uint8_t ciphertext[CLASS_SIZE+MAC_LEN];
} private_firev_data_t;


#endif