//
// Created by Dananjaya RAMANAYAKE on 19/11/2023.
//
#include "bme280_port.h"

extern I2C_HandleTypeDef hi2c1;

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 *
 * @param[in] reg_addr       : Register address from which data is read.
 * @param[out] reg_data      : Pointer to data buffer where read data is stored.
 * @param[in] len            : Number of bytes of data to be read.
 * @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                             for interface related call backs.
 *
 * @retval   0 -> Success.
 * @retval Non zero value -> Fail.
 *
 */
BME280_INTF_RET_TYPE bme280_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    HAL_StatusTypeDef status = HAL_OK;
    int8_t error = 0;

    uint8_t array[I2C_BUFFER_LEN] = {0};
    uint8_t stringpos = 0;
    array[0] = reg_addr;

    while (HAL_I2C_IsDeviceReady(&hi2c1, BME280_ADDR, 3, 100) != HAL_OK) {}

    status = HAL_I2C_Mem_Read(&hi2c1,						// i2c handle
                              BME280_ADDR,		// i2c address, left aligned
                              reg_addr,			// register address
                              I2C_MEMADD_SIZE_8BIT,			// bme280 uses 8bit register addresses
                              (uint8_t *)&array,			// write returned data to this variable
                              len,							// how many bytes to expect returned
                              100);							// timeout

    if(status == HAL_OK) {
        for (stringpos = 0; stringpos < len; stringpos++) {
            *(reg_data + stringpos) = array[stringpos];
        }
        return status;
    } else {
        // The BME280 API calls for 0 return value as a success, and -1 returned as failure
        error = (-1);
    }

    return error;
}

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific write functions of the user
 *
 * @param[in] reg_addr      : Register address to which the data is written.
 * @param[in] reg_data      : Pointer to data buffer in which data to be written
 *                            is stored.
 * @param[in] len           : Number of bytes of data to be written.
 * @param[in, out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                            for interface related call backs
 *
 * @retval   0   -> Success.
 * @retval Non zero value -> Fail.
 *
 */
BME280_INTF_RET_TYPE bme280_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    HAL_StatusTypeDef status = HAL_OK;
    int8_t error = 0;

    while (HAL_I2C_IsDeviceReady(&hi2c1, (uint8_t)(BME280_ADDR), 3, 100) != HAL_OK) {}

    status = HAL_I2C_Mem_Write(&hi2c1,						// i2c handle
                               (uint8_t)(BME280_ADDR),		// i2c address, left aligned
                               (uint8_t)reg_addr,			// register address
                               I2C_MEMADD_SIZE_8BIT,			// bme280 uses 8bit register addresses
                               (uint8_t*)(&reg_data),		// write returned data to reg_data
                               len,							// write how many bytes
                               100);							// timeout

    if (status != HAL_OK)
    {
        // The BME280 API calls for 0 return value as a success, and -1 returned as failure
        error = (-1);
    }
    return error;
}

/*!
 * @brief Delay function pointer which should be mapped to
 * delay function of the user
 *
 * @param[in] period              : Delay in microseconds.
 * @param[in, out] intf_ptr       : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *
 */
void bme280_delay(uint32_t period, void *intf_ptr) {
    HAL_Delay(period / 1000);
}

