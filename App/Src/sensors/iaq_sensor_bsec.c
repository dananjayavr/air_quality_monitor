//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//
#include "main.h"
#include "bsec_integration.h"
#include "bsec_selectivity.h"
#include "iaq_sensor_bsec.h"
#include "ssd1306.h"

extern sensor_data_t sensor_data_all;

/**********************************************************************************************************************/
/* functions */
/**********************************************************************************************************************/
void DelayUs(uint32_t Delay)
{
    uint32_t i;

    while(Delay--)
    {
        for(i = 0; i < 168; i++)
        {
            ;
        }
    }
}

/*!
 * @brief           System specific implementation of sleep function
 *
 * @param[in]       t_ms    time in milliseconds
 *
 * @return          none
 */
void sleep(uint32_t t_ms, void *intf_ptr)
{
    HAL_Delay(t_ms);
    //DelayUs(t_ms*1000);
}

/*!
 * @brief           Capture the system time in microseconds
 *
 * @return          system_current_time    current system timestamp in microseconds
 */
int64_t get_timestamp_us()
{
    int64_t system_current_time = 0;
    // Get the current value of the microsecond counter
    system_current_time =  HAL_GetTick() * 1000;

    return system_current_time;
}

/*!
 * @brief           Handling of the ready outputs
 *
 * @param[in]       timestamp               time in milliseconds
 * @param[in]       gas_estimate_1          gas estimate 1
 * @param[in]       gas_estimate_2          gas estimate 2
 * @param[in]       gas_estimate_3          gas estimate 3
 * @param[in]       gas_estimate_4          gas estimate 4
 * @param[in]       raw_pressure            raw pressure
 * @param[in]       raw_temp                raw temperature
 * @param[in]       raw_humidity            raw humidity
 * @param[in]       raw_gas                 raw gas
 * @param[in]       raw_gas_index           raw gas index
 * @param[in]       bsec_status             value returned by the bsec_do_steps() call
 *
 * @return          none
 */
#if 0
void output_ready(int64_t timestamp, float gas_estimate_1, float gas_estimate_2, float gas_estimate_3, float gas_estimate_4,
                  float raw_pressure, float raw_temp, float raw_humidity, float raw_gas, uint8_t raw_gas_index, bsec_library_return_t bsec_status)
{
    // ...
    // Please insert system specific code to further process or display the BSEC outputs
    // ...
    TRACE_INFO("Timestamp: %lld, Gas #1: %f, Gas #3: %f, Gas #3: %f, Gas #4: %f Pressure: %f, Temperature: %f, Humidity: %f, Gas (Raw): %f, Gas Index: %hhu\r\n",
               timestamp, gas_estimate_1,gas_estimate_2,gas_estimate_3,gas_estimate_4,raw_pressure,raw_temp,raw_humidity,raw_gas,raw_gas_index);
}
#endif
void output_ready(int64_t timestamp, float iaq, float iaq_accuracy, float temp, float humidity,
                                      float raw_pressure, float raw_temp, float static_iaq, float co2_equivalent, uint8_t breath_voc_equivalent, bsec_library_return_t bsec_status) {
    char buffer[32];

    TRACE_DEBUG("Status: %d:\tIAQ: %f, IAQ Accuracy: %f, Temperature: %f, Humidity: %f, Raw Pressure: %f, "
               "Raw Temperature: %f, Static IAQ: %f, CO2 Equivalent: %f, Breath CO2 Equivalent: %d\r\n",
               bsec_status,
               iaq,
               iaq_accuracy,
               temp,
               humidity,
               raw_pressure,raw_temp,static_iaq,co2_equivalent, breath_voc_equivalent);

    sensor_data_all.iaq = iaq;
    sensor_data_all.accuracy = iaq_accuracy;
    sensor_data_all.co2_eq = co2_equivalent;
    sensor_data_all.breath_voc = breath_voc_equivalent;

    ssd1306_SetCursor(0, 90);
    if(iaq_accuracy == 0 || iaq_accuracy == 1) {
        sprintf(buffer, "Bosch IAQ Index.**"); // Need calibration
    } else if (iaq_accuracy == 2) {
        sprintf(buffer, "Bosch IAQ Index.*"); // Calibration might improve accuracy
    } else if (iaq_accuracy == 3) {
        sprintf(buffer, "Bosch IAQ Index"); // high accuracy
    }

    ssd1306_WriteString(buffer, Font_7x10, White);
    memset(buffer,0,32);

    if(iaq >= 0 && iaq <= 100) {
        sprintf(buffer,"%.0f (Good)",iaq);
    } else if (iaq >= 101 && iaq <= 150) {
        sprintf(buffer,"%.0f (Fair)",iaq);
    } else {
        sprintf(buffer,"%.0f (Bad)",iaq);
    }

    ssd1306_SetCursor(0, 100);
    ssd1306_WriteString(buffer, Font_11x18, White);

    ssd1306_UpdateScreen();

}
/*!
 * @brief           Load previous library state from non-volatile memory
 *
 * @param[in,out]   state_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to state_buffer
 */
uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer)
{
    // ...
    // Load a previous library state from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no state was available,
    // otherwise return length of loaded state string.
    // ...
    return 0;
}

/*!
 * @brief           Save library state to non-volatile memory
 *
 * @param[in]       state_buffer    buffer holding the state to be stored
 * @param[in]       length          length of the state string to be stored
 *
 * @return          none
 */
void state_save(const uint8_t *state_buffer, uint32_t length)
{
    // ...
    // Save the string some form of non-volatile memory, if possible.
    // ...
}

/*!
 * @brief           Load library config from non-volatile memory
 *
 * @param[in,out]   config_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to config_buffer
 */
uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer)
{
    // ...
    // Load a library config from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no config was available,
    // otherwise return length of loaded config string.
    // ...
    n_buffer = 1974;
    memcpy(config_buffer,bsec_config_selectivity,n_buffer);

    return n_buffer;
}

void bme688_bsec_init_sensor(void) {
    return_values_init ret = {0};
    struct bme68x_dev bme_dev;

    memset(&bme_dev,0,sizeof(bme_dev));

    bme_dev.read =  bme68x_i2c_read;
    bme_dev.write = bme68x_i2c_write;
    bme_dev.intf =  BME68X_I2C_INTF;
    bme_dev.delay_us = bme68x_delay_us;
    bme_dev.amb_temp = 25; /* The ambient temperature in deg C is used for defining the heater temperature */

    ret = bsec_iot_init(BSEC_SAMPLE_RATE_LP,0.0f,bme68x_i2c_write,
                  bme68x_i2c_read,sleep,state_load,config_load,bme_dev);

    if (ret.bme68x_status != BME68X_OK)
    {
        /* Could not intialize BME680 */
        TRACE_INFO("Could not intialize BSEC library, bme680_status=%d\r\n", ret.bme68x_status);
    }
    else if (ret.bsec_status != BSEC_OK)
    {
        /* Could not intialize BSEC library */
        TRACE_INFO("Could not intialize BSEC library, bsec_status=%d\r\n", ret.bsec_status);
    }
}

void bme688_bsec_read_sensor(void) {
    /* Call to endless loop function which reads and processes data based on sensor settings */
    /* State is saved every 10.000 samples, which means every 10.000 * 3 secs = 500 minutes  */
    bsec_iot_loop(sleep, get_timestamp_us, output_ready, state_save, 10000);
}

