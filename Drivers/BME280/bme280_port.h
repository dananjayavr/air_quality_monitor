//
// Created by Dananjaya RAMANAYAKE on 19/11/2023.
//

#include "stm32f4xx_hal.h"
#include "bme280.h"

#define BME280_ADDR 0x77 << 1
#define	I2C_BUFFER_LEN 32

BME280_INTF_RET_TYPE bme280_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

BME280_INTF_RET_TYPE bme280_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);

void bme280_delay(uint32_t period, void *intf_ptr);