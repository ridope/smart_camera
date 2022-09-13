/** @file amp_send.h
 *  @brief Header file that controls the AMP cores communication
 *
 *  @author Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
 */

#ifndef AMP_SEND_H
#define AMP_SEND_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../core_1_firmware/data.h"

void amp_send_init(void);
void amp_send_class(uint8_t p_class);

#endif