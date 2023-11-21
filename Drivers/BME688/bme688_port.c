//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#include "bme688_port.h"

extern I2C_HandleTypeDef hi2c1;

/*!
 * I2C read function map to STM32
 */
BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    uint8_t array[32] = { 0 };
    uint8_t stringpos = 0;
    array[0] = reg_addr;

    UNUSED(intf_ptr);

    while (HAL_I2C_IsDeviceReady(&hi2c1, BME688_ADDR, 3, 100)!= HAL_OK) {}

    status = HAL_I2C_Mem_Read(&hi2c1,	    // I2C handle
                              BME688_ADDR,					// I2C address, left aligned
                              reg_addr,			// register address
                              I2C_MEMADD_SIZE_8BIT,	    // BME688 uses 8bit register addresses
                              (uint8_t*) (&array),		// write returned data to this variable
                              len,						// how many bytes to expect returned
                              100);					    // timeout

    if(status == HAL_OK) {
        for (stringpos = 0; stringpos < len; stringpos++) {
            *(reg_data + stringpos) = array[stringpos];
        }
        return status;
    } else {
        return -1;
    }
}

/*!
 * I2C write function map to STM32
 */
BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;

    UNUSED(intf_ptr);

    while (HAL_I2C_IsDeviceReady(&hi2c1, BME688_ADDR, 3, 100)!= HAL_OK) {}

    status = HAL_I2C_Mem_Write(&hi2c1,	    // I2C handle
                               BME688_ADDR,		            // I2C address, left aligned
                               reg_addr,			// register address
                               I2C_MEMADD_SIZE_8BIT,	    // BME688 uses 8bit register addresses
                               (uint8_t*) (&reg_data),		// write returned data to reg_data
                               len,			        	// write how many bytes
                               100);					    // timeout

    if(status == HAL_OK) {
        return 0;
    } else {
        return -1;
    }
}

void bme68x_delay_us(uint32_t period, void *intf_ptr)
{
    UNUSED(intf_ptr);
#if 0
    if(period % 1000 == 0) {
        HAL_Delay(period/1000);
    } else {
        HAL_Delay(100);
    }
#endif
    HAL_Delay(period);
}
