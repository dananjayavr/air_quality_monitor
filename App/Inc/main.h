#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "stm32f4xx_hal.h"
#include "debug.h"
#include "console.h"
#include "retarget.h"
#include "bme280_port.h"

#define FW_VERSION "Current firmware: v0.0.1"
#define SGP30_ENABLED 1

typedef struct calibration_data_unpacked_s
{
    uint16_t sgp30;
    uint16_t bme688;
} calibration_data_unpacked_t;

typedef struct __attribute__ ((__packed__)) calibration_data_s
{
    uint16_t sgp30;
    uint16_t bme688;
} calibration_data_t;

typedef union message_u
{
    calibration_data_t data;
    uint8_t bytes[sizeof(calibration_data_t)];
} calibration_t;


typedef struct  {
    uint16_t PM1_0_atmospheric; // concentration unit * μg/m3 - under atmospheric environment
    uint16_t PM2_5_atmospheric;
    uint16_t PM10_atmospheric;

    double pressure;
    double temperature;
    double humidity;

    uint16_t tvoc_ppb;
    uint16_t co2_eq_ppm;

    float iaq;
    float accuracy;
    float co2_eq;
    float breath_voc;
} sensor_data_t;

void Error_Handler(void);
void printSensorData(sensor_data_t data);

#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB

#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB

#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB

#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC

#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD

#define TIMER_TEST_PIN_Pin GPIO_PIN_12
#define TIMER_TEST_PIN_GPIO_Port GPIOD

#endif //_MAIN_H
