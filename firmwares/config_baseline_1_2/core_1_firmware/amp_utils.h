/** @file amp_utils.h
 *  @brief Header file with utility fuctions for the AMP cores
 *
 *  @author Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
 */

#include <generated/csr.h>

#ifndef AMP_UTILS_H
#define AMP_UTILS_H

void amp_millis_init(void);
uint32_t amp_millis(void);

#endif