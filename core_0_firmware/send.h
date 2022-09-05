/** @file amp_send.h
 *  @brief Header file that controls the AMP cores communication
 *
 *  @author Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
 */

#ifndef SEND_H
#define SEND_H

#include <stdio.h>
#include <stdlib.h>
#include "../core_1_firmware/data.h"

void amp_send_class(uint8_t class);

#endif