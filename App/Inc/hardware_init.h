//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#ifndef _HARDWARE_INIT_H
#define _HARDWARE_INIT_H

void SystemClock_Config(void);

void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);
void MX_UART5_Init(void);
void MX_TIM2_Init(void);
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);

#endif //_HARDWARE_INIT_H
