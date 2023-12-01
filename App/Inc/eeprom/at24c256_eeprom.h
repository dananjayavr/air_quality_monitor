//
// Created by Dananjaya RAMANAYAKE on 01/12/2023.
//

#ifndef _AT24C256_PORT_H
#define _AT24C256_PORT_H

#include "stm32f4xx_hal.h"

void EEPROM_Write (uint16_t page, uint16_t offset, uint8_t *data, uint16_t size);
void EEPROM_Read (uint16_t page, uint16_t offset, uint8_t *data, uint16_t size);
void EEPROM_PageErase (uint16_t page);
void EEPROM_AllPageErase (void);

#endif //_AT24C256_PORT_H
