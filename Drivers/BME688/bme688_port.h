//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#ifndef _BME688_PORT_H
#define _BME688_PORT_H

#include "stm32f4xx_hal.h"
#include "bme68x.h"

#define BME688_ADDR 0x76 << 1

BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len,void *intf_ptr);
BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len,void *intf_ptr);

void bme68x_delay_us(uint32_t period, void *intf_ptr);

#endif //_BME688_PORT_H
