//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#include "main.h"
#include "bme688_port.h"
#include "iaq_sensor.h"

struct bme68x_dev bme ={0};
struct bme68x_data data = {0};
struct bme68x_conf conf ={0};
struct bme68x_heatr_conf heatr_conf = {0};

uint32_t del_period;
uint8_t n_fields;

void bme688_init_sensor(void) {
    int8_t bme688_rslt = BME68X_OK;

    bme.read =  bme68x_i2c_read;
    bme.write = bme68x_i2c_write;
    bme.intf =  BME68X_I2C_INTF;

    bme.delay_us = bme68x_delay_us;
    bme.amb_temp = 25; /* The ambient temperature in deg C is used for defining the heater temperature */

    do {
        bme688_rslt = bme68x_init(&bme);
        if(bme688_rslt < 0) {
            TRACE_INFO("BME688 Init Failed.\r\n");
            bme688_error_codes_print_result("bme68x_init", bme688_rslt);
            break;
        }

        conf.filter = BME68X_FILTER_OFF;
        conf.odr = BME68X_ODR_20_MS;//BME68X_ODR_NONE;
        conf.os_hum = BME68X_OS_16X;
        conf.os_pres = BME68X_OS_1X;
        conf.os_temp = BME68X_OS_2X;
        bme688_rslt = bme68x_set_conf(&conf, &bme);
        if(bme688_rslt < 0) {
            TRACE_INFO("BME688 Set Config Failed.\r\n");
            bme688_error_codes_print_result("bme68x_set_conf", bme688_rslt);
            break;
        }

        heatr_conf.enable = BME68X_ENABLE;
        heatr_conf.heatr_temp = 300;
        heatr_conf.heatr_dur = 100;
        bme688_rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
        if(bme688_rslt < 0) {
            TRACE_INFO("BME688 Set Heater Config Failed.\r\n");
            bme688_error_codes_print_result("bme68x_set_heatr_conf", bme688_rslt);
            break;
        }

    } while (0);
}

void bme688_read_sensor(void) {
    int8_t bme688_rslt = BME68X_OK;

    do {
        bme688_rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
        if(bme688_rslt < 0) {
            TRACE_INFO("BME688 Set OP Mode Failed.\r\n");
            bme688_error_codes_print_result("bme68x_set_op_mode", bme688_rslt);
            break;
        }

        /* Calculate delay period in microseconds */
        del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme) + (heatr_conf.heatr_dur * 1000);
        bme.delay_us(del_period, bme.intf_ptr);
        /* Check if rslt == BME68X_OK, report or handle if otherwise */
        bme688_rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);
        if(bme688_rslt < 0) {
            TRACE_INFO("BME688 Set OP Mode Failed.\r\n");
            bme688_error_codes_print_result("bme68x_get_data", bme688_rslt);
            break;
        }
    } while(0);

    if (n_fields) {
        TRACE_INFO("%.2f, %.2f, %.2f, %.2f, 0x%x\n",
               data.temperature,
               data.pressure,
               data.humidity,
               data.gas_resistance,
               data.status);
    } else {
        TRACE_INFO("N-Fields=0\r\n");
    }
}

void bme688_error_codes_print_result(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BME68X_OK:
            TRACE_INFO("API name [%s]  Error [%d] : Sensor Init OK.\r\n", api_name, rslt);
            /* Do nothing */
            break;
        case BME68X_E_NULL_PTR:
            TRACE_INFO("API name [%s]  Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BME68X_E_COM_FAIL:
            TRACE_INFO("API name [%s]  Error [%d] : Communication failure\r\n", api_name, rslt);
            break;
        case BME68X_E_INVALID_LENGTH:
            TRACE_INFO("API name [%s]  Error [%d] : Incorrect length parameter\r\n", api_name, rslt);
            break;
        case BME68X_E_DEV_NOT_FOUND:
            TRACE_INFO("API name [%s]  Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        case BME68X_E_SELF_TEST:
            TRACE_INFO("API name [%s]  Error [%d] : Self test error\r\n", api_name, rslt);
            break;
        case BME68X_W_NO_NEW_DATA:
            TRACE_INFO("API name [%s]  Warning [%d] : No new data found\r\n", api_name, rslt);
            break;
        default:
            TRACE_INFO("API name [%s]  Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}
