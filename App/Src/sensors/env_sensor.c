//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#include "env_sensor.h"

int8_t bme280_rslt = 0;
uint8_t bme280_status_reg;
uint32_t bme280_period = 0;
struct bme280_dev bme280;
struct bme280_settings bme280Settings;
struct bme280_data bme280_comp_data;

void bme280_init_sensor(void) {

    // BME280 Initialization
    bme280.intf = BME280_I2C_INTF;
    bme280.write = bme280_write;
    bme280.read = bme280_read;
    bme280.delay_us = bme280_delay;

    bme280.intf_ptr = &bme280;

    do {
        bme280_rslt = bme280_init(&bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Init Failed.\r\n");
            bme280_error_codes_print_result("bme280_init", bme280_rslt);
            break;
        }

        bme280_rslt = bme280_get_sensor_settings(&bme280Settings,&bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Get Sensor settings failed.\r\n");
            bme280_error_codes_print_result("bme280_get_sensor_settings", bme280_rslt);
            break;
        }

        /* Configuring the over-sampling rate, filter coefficient and standby time */
        /* Overwrite the desired settings */
        /* Over-sampling rate for humidity, temperature and pressure */
        bme280Settings.osr_h = BME280_OVERSAMPLING_1X;
        bme280Settings.osr_p = BME280_OVERSAMPLING_1X;
        bme280Settings.osr_t = BME280_OVERSAMPLING_1X;
        bme280Settings.filter = BME280_FILTER_COEFF_OFF;

        /* Setting the standby time */
        bme280Settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

        bme280_rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &bme280Settings, &bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Set Sensor settings failed.\r\n");
            bme280_error_codes_print_result("bme280_set_sensor_settings", bme280_rslt);
            break;
        }

        /* Always set the power mode after setting the configuration */
        bme280_rslt = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Set Sensor mode failed.\r\n");
            bme280_error_codes_print_result("bme280_set_sensor_mode", bme280_rslt);
            break;
        }

        bme280.delay_us(40,bme280.intf_ptr);

        /* Calculate measurement time in microseconds */
        bme280_rslt = bme280_cal_meas_delay(&bme280_period, &bme280Settings);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 calculate measurement time failed.\r\n");
            bme280_error_codes_print_result("bme280_cal_meas_delay", bme280_rslt);
            break;
        }

        TRACE_DEBUG("\nTemperature calculation (Data displayed are compensated values)\r\n");
        TRACE_DEBUG("Measurement time : %lu us\r\n", (long unsigned int)bme280_period);

    } while (0);
}

void bme280_read_sensor(void) {
    // Get BME280 data
    bme280_rslt = bme280_get_regs(BME280_REG_STATUS, &bme280_status_reg, 1, &bme280);
    if(bme280_rslt < 0) {
        TRACE_INFO("BME280 Get regs failed.\r\n");
    }

    if (1/*bme280_status_reg & BME280_STATUS_MEAS_DONE*/)
    {
        /* Measurement time delay given to read sample */
        bme280.delay_us(bme280_period, bme280.intf_ptr);

        /* Read compensated data */
        bme280_rslt = bme280_get_sensor_data(BME280_ALL, &bme280_comp_data, &bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Get sensor data failed.\r\n");
        }

        TRACE_INFO("BME280:\r\n   "
                   "\tTemperature: %.3f deg C\r\n\tPressure: %.3f hPa\r\n\tHumidity: %.3f%%\r\n", bme280_comp_data.temperature,
                   0.01 * bme280_comp_data.pressure,bme280_comp_data.humidity
        );

        /* set sensor mode for subsequent reads */
        bme280_rslt = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Set Sensor mode failed.\r\n");
            bme280_error_codes_print_result("bme280_set_sensor_mode", bme280_rslt);
        }
    }
}

/*!
 *  @brief Prints the execution status of the APIs.
 */
void bme280_error_codes_print_result(const char api_name[], int8_t rslt)
{
    if (rslt != BME280_OK)
    {
        printf("%s\t", api_name);

        switch (rslt)
        {
            case BME280_E_NULL_PTR:
                printf("Error [%d] : Null pointer error.", rslt);
                printf(
                        "It occurs when the user tries to assign value (not address) to a pointer, which has been initialized to NULL.\r\n");
                break;

            case BME280_E_COMM_FAIL:
                printf("Error [%d] : Communication failure error.", rslt);
                printf(
                        "It occurs due to read/write operation failure and also due to power failure during communication\r\n");
                break;

            case BME280_E_DEV_NOT_FOUND:
                printf("Error [%d] : Device not found error. It occurs when the device chip id is incorrectly read\r\n",
                       rslt);
                break;

            case BME280_E_INVALID_LEN:
                printf("Error [%d] : Invalid length error. It occurs when write is done with invalid length\r\n", rslt);
                break;

            default:
                printf("Error [%d] : Unknown error code\r\n", rslt);
                break;
        }
    }
}
