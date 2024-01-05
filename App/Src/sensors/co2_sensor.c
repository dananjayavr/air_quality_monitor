//
// Created by Dananjaya RAMANAYAKE on 22/11/2023.
//
#include <inttypes.h>
#include <string.h>
#include "main.h"
#include "co2_sensor.h"
#include "ssd1306.h"

uint16_t tvoc_ppb, co2_eq_ppm;
uint32_t iaq_baseline;
uint16_t ethanol_raw_signal, h2_raw_signal;
uint16_t i = 0;

extern sensor_data_t sensor_data_all;

void sgp30_init_sensor(void) {

    int16_t err;

    const char* driver_version = sgp30_get_driver_version();
    if (driver_version) {
        TRACE_INFO("SGP30 driver version %s\r\n", driver_version);
    } else {
        TRACE_INFO("fatal: Getting driver version failed\r\n");
    }

    /* Initialize I2C bus */
    sensirion_i2c_init();

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    int16_t probe;
    while (1) {
        probe = sgp30_probe();

        if (probe == STATUS_OK)
            break;

        if (probe == SGP30_ERR_UNSUPPORTED_FEATURE_SET)
            TRACE_INFO(
                    "Your sensor needs at least feature set version 1.0 (0x20)\r\n");

        TRACE_INFO("SGP sensor probing failed\r\n");
        sensirion_sleep_usec(1000000);
    }

    TRACE_INFO("SGP sensor probing successful\r\n");

    uint16_t feature_set_version;
    uint8_t product_type;
    err = sgp30_get_feature_set_version(&feature_set_version, &product_type);
    if (err == STATUS_OK) {
        TRACE_INFO("Feature set version: %u\r\n", feature_set_version);
        TRACE_INFO("Product type: %u\r\n", product_type);
    } else {
        TRACE_INFO("sgp30_get_feature_set_version failed!\r\n");
    }
    uint64_t serial_id;
    err = sgp30_get_serial_id(&serial_id);
    if (err == STATUS_OK) {
        //TRACE_INFO("SerialID: %" PRIu64 "\r\n", serial_id);
    } else {
        TRACE_INFO("sgp30_get_serial_id failed!\r\n");
    }
}

void sgp30_read_sensor(void) {
    int16_t err;
    char buffer[32];

    /* Read gas raw signals */
    err = sgp30_measure_raw_blocking_read(&ethanol_raw_signal, &h2_raw_signal);
    if (err == STATUS_OK) {
        /* Print ethanol raw signal and h2 raw signal */
        //TRACE_INFO("Ethanol raw signal: %u\n", ethanol_raw_signal);
        //TRACE_INFO("H2 raw signal: %u\n", h2_raw_signal);
    } else {
        TRACE_INFO("error reading raw signals\n");
    }

    /* Consider the two cases (A) and (B):
     * (A) If no baseline is available or the most recent baseline is more than
     *     one week old, it must discarded. A new baseline is found with
     *     sgp30_iaq_init() */
    err = sgp30_iaq_init();
    if (err == STATUS_OK) {
        //TRACE_INFO("sgp30_iaq_init done\r\n");
    } else {
        TRACE_INFO("sgp30_iaq_init failed!\r\n");
    }
    /* (B) If a recent baseline is available, set it after sgp30_iaq_init() for
     * faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgp30_set_iaq_baseline(iaq_baseline);
     */

    /* Run periodic IAQ measurements at defined intervals */
    /*
         * IMPLEMENT: get absolute humidity to enable humidity compensation
         * uint32_t ah = get_absolute_humidity(); // absolute humidity in mg/m^3
         * sgp30_set_absolute_humidity(ah);
         */

    err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
    if (err == STATUS_OK) {
        //TRACE_INFO("tVOC  Concentration: %dppb\n", tvoc_ppb);
        //TRACE_INFO("CO2eq Concentration: %dppm\n", co2_eq_ppm);
    } else {
        TRACE_INFO("error reading IAQ values\n");
    }

    /* Persist the current baseline every hour */
    if (++i % 3600 == 3599) {
        err = sgp30_get_iaq_baseline(&iaq_baseline);
        if (err == STATUS_OK) {
            /* IMPLEMENT: store baseline to presistent storage */
        }
    }

    TRACE_DEBUG("SGP30:\r\n   "
               "\tEthanol Raw Signal: %u\r\n\tH2 Raw Signal: %u\r\n\t"\
               "tVOC Concentration: %d ppb\r\n\tCO2eq Concentration: %d ppm\r\n", ethanol_raw_signal,
               h2_raw_signal,tvoc_ppb,co2_eq_ppm);

    sensor_data_all.co2_eq = co2_eq_ppm;
    sensor_data_all.tvoc_ppb = tvoc_ppb;

#if 0
    ssd1306_SetCursor(0, 60);
    sprintf(buffer, "tVOC: %d ppb", tvoc_ppb);
    ssd1306_WriteString(buffer, Font_7x10, White);
    memset(buffer,0,32);
    ssd1306_SetCursor(0, 70);
    sprintf(buffer, "CO2eq: %d ppm", co2_eq_ppm);
    ssd1306_WriteString(buffer, Font_7x10, White);
#endif
    ssd1306_UpdateScreen();

    /* The IAQ measurement must be triggered exactly once per second (SGP30)
     * to get accurate values.
     */
    //sleep(1);  // SGP30
}
