#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "debug.h"
#include "console.h"
#include "retarget.h"
#include "bme280_port.h"

typedef struct message_data_unpacked_s
{
    uint16_t time;
    uint16_t lat;
    uint8_t ns;
    uint16_t lon;
    uint8_t ew;
} message_data_unpacked_t;

typedef struct __attribute__ ((__packed__)) message_data_s
{
    uint16_t time;
    uint16_t lat;
    uint8_t ns;
    uint16_t lon;
    uint8_t ew;
} message_data_t;

typedef union message_u
{
    message_data_t data;
    uint8_t bytes[sizeof(message_data_t)];
} message_t;

void Error_Handler(void);

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

#define BSEC_TIMER_TEST_PIN_Pin GPIO_PIN_12
#define BSEC_TIMER_TEST_PIN_GPIO_Port GPIOD

#endif //_MAIN_H
