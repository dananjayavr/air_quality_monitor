//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//
#include <string.h>
#include "main.h"
#include "bme688_port.h"
#include "iaq_sensor.h"
#include "bsec_interface.h"
#include "bsec_selectivity.h"
#include "ssd1306.h"

struct bme68x_dev bme ={0};
struct bme68x_data data = {0};
struct bme68x_conf conf ={0};
struct bme68x_heatr_conf heatr_conf = {0};

uint32_t del_period;
uint8_t n_fields;

/* Define which sensor values to request.
 * The order is not important, but output_ready needs to be updated if different types
 * of sensor values are requested.
 */
const bsec_sensor_configuration_t bsec_requested_virtual_sensors[4] = {
        {
                .sensor_id   = BSEC_OUTPUT_IAQ,
                .sample_rate = BSEC_SAMPLE_RATE_LP,
        },
        {
                .sensor_id   = BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
                .sample_rate = BSEC_SAMPLE_RATE_LP,
        },
        {
                .sensor_id   = BSEC_OUTPUT_RAW_PRESSURE,
                .sample_rate = BSEC_SAMPLE_RATE_LP,
        },
        {
                .sensor_id   = BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
                .sample_rate = BSEC_SAMPLE_RATE_LP,
        },
};

void bme688_init_sensor(void) {
    int8_t bme688_rslt = BME68X_OK;
    bsec_library_return_t bsec_rslt = 0;

    bsec_sensor_configuration_t required_sensor_settings[BSEC_MAX_PHYSICAL_SENSOR];
    uint8_t n_required_sensor_settings = BSEC_MAX_PHYSICAL_SENSOR;

    bsec_bme_settings_t run_gas[1];

    bme.read =  bme68x_i2c_read;
    bme.write = bme68x_i2c_write;
    bme.intf =  BME68X_I2C_INTF;

    bme.delay_us = bme68x_delay_us;
    bme.amb_temp = 25; /* The ambient temperature in deg C is used for defining the heater temperature */

    uint8_t work_buffer_state[BSEC_MAX_WORKBUFFER_SIZE];
    uint32_t  n_work_buffer_size = BSEC_MAX_WORKBUFFER_SIZE;

    do {
        bme688_rslt = bme68x_init(&bme);
        if(bme688_rslt < 0) {
            TRACE_INFO("BME688 Init Failed.\r\n");
            bme688_error_codes_print_result("bme68x_init", bme688_rslt);
            break;
        }

        conf.filter = BME68X_FILTER_OFF;
        conf.odr = BME68X_ODR_NONE;
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

    do {
        bsec_rslt = bsec_init();
        if(bsec_rslt != BSEC_OK) {
            TRACE_INFO("BSEC library init failed.\r\n");
            break;
        }

        bsec_rslt = bsec_set_state(bsec_config_selectivity,1974,
                                   work_buffer_state,n_work_buffer_size);
        if(bsec_rslt != BSEC_OK) {
            TRACE_INFO("BSEC library set state failed.\r\n");
            break;
        }

        // Call bsec_update_subscription() to enable/disable the requested virtual sensors
        bsec_rslt = bsec_update_subscription(bsec_requested_virtual_sensors, 4,
                                 required_sensor_settings, &n_required_sensor_settings);
        if(bsec_rslt != BSEC_OK) {
            TRACE_INFO("BSEC library update subscription failed.\r\n");
            break;
        }

        run_gas[0].heater_temperature = 300;
        run_gas[0].heater_duration = 100;

        bsec_rslt = bsec_sensor_control(HAL_GetTick()/1000,run_gas);
        if(bsec_rslt != BSEC_OK) {
            TRACE_INFO("BSEC library update subscription failed.\r\n");
            break;
        }

    } while (0);
}

void bme688_read_sensor(void) {
    int8_t bme688_rslt = BME68X_OK;
    //char buffer[32];

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
        TRACE_DEBUG("BME688:\r\n   "
                   "\tTemperature: %.3f deg C\r\n\tPressure: %.3f hPa\r\n\tHumidity: %.2f%%\r\n\tGas resistance: %.2f Ohm\r\n\tStatus: 0x%x\r\n",
                   data.temperature,
                   data.pressure * 0.01,data.humidity,data.gas_resistance,data.status);
#if 0
        ssd1306_SetCursor(0, 110);
        sprintf(buffer, "Gas: %.2f Ohm", data.gas_resistance);
        ssd1306_WriteString(buffer, Font_7x10, White);
        ssd1306_UpdateScreen();
#endif

    } else {
        TRACE_INFO("N-Fields=0\r\n");
    }

    bme688_rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
    if(bme688_rslt < 0) {
        TRACE_INFO("BME688 Set OP Mode Failed.\r\n");
        bme688_error_codes_print_result("bme68x_set_op_mode", bme688_rslt);
    }
}

void bme688_iaq_algo(void) {
    char buffer[32];
    // Allocate input and output memory
    bsec_input_t input[3];
    uint8_t n_input = 3;
    bsec_output_t output[3];
    uint8_t  n_output=3;

    bsec_library_return_t status;

    float gas_r = data.gas_resistance;
    float temp = data.temperature;
    float hum = data.humidity;
    uint32_t timestamp = HAL_GetTick() / 1000;

    float iaq = 0.0f;
    float static_iaq = 0.0f;
    float accuracy = 0.0f;

    // Populate the input structs, assuming the we have timestamp (ts),
    // gas sensor resistance (R), temperature (T), and humidity (rH) available
    // as input variables
    input[0].sensor_id = BSEC_INPUT_GASRESISTOR;
    input[0].signal = gas_r;
    input[0].time_stamp= timestamp;
    input[1].sensor_id = BSEC_INPUT_TEMPERATURE;
    input[1].signal = temp;
    input[1].time_stamp= timestamp;
    input[2].sensor_id = BSEC_INPUT_HUMIDITY;
    input[2].signal = hum;
    input[2].time_stamp= timestamp;

    // Invoke main processing BSEC function
    status = bsec_do_steps( input, n_input, output, &n_output );

    // Iterate through the BSEC output data, if the call succeeded
    if(status == BSEC_OK)
    {
        for(int i = 0; i < n_output; i++)
        {
            switch(output[i].sensor_id)
            {
                case BSEC_OUTPUT_IAQ:
                    iaq = output[i].signal;
                    accuracy = output[i].accuracy;
                    // Retrieve the IAQ results from output[i].signal
                    // and do something with the data
                    TRACE_DEBUG("IAQ: %f\r\n",iaq);
                    break;
                case BSEC_OUTPUT_STATIC_IAQ:
                    static_iaq = output[i].signal;
                    accuracy = output[i].accuracy;
                    // Retrieve the static IAQ results from output[i].signal
                    // and do something with the data
                    TRACE_INFO("Static IAQ: %f (%f)\r\n", static_iaq, accuracy);
                    break;
            }

            ssd1306_SetCursor(0, 110);
            sprintf(buffer, "IAQ: %.2f (%.2f)", iaq, accuracy);
            ssd1306_WriteString(buffer, Font_7x10, White);
            ssd1306_UpdateScreen();
        }

        TRACE_INFO("Gas Resistance: %.2f\r\n",data.gas_resistance);
    }
    else
    {
        TRACE_INFO("BME688 IAQ algo failed.\r\n");
    }

    HAL_Delay(2000);
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
