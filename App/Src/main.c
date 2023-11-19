#include <stdio.h>
#include <string.h>
#include "main.h"
#include "debug.h"
#include "console.h"
#include "retarget.h"
#include "PMS5003_HAL_STM32.h"
#include "bme280_port.h"

#define TRACE_LEVEL TRACE_LEVEL_INFO

// 5x5 RGB : I2C Addr 0x74
// BME688 : I2C Addr 0x76
// BME280 : I2C Addr 0x77
// SGP30 : I2C Addr 0x58

volatile uint8_t pb_state; // hold push button state
volatile uint8_t pb_toggle; // hold toggled push button state

UART_HandleTypeDef huart2; // UART console
UART_HandleTypeDef huart3; // printf redirect
UART_HandleTypeDef huart5; // PMS5003 particulate matter sensor
TIM_HandleTypeDef htim2;
I2C_HandleTypeDef hi2c1;

PMS_typedef PMS5003 = {0};
char mesg[1000] = {0};

void Error_Handler(void);
void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART5_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);

void sensor_init(void);
void bme280_error_codes_print_result(const char api_name[], int8_t rslt);



/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // Making sure interrupt is coming from the GPIO_PIN_13 (our push button)
    if(GPIO_Pin == USER_Btn_Pin) {
        pb_state = 1;
        HAL_TIM_Base_Start_IT(&htim2); // Start debounce timer
    } else {
        __NOP();
    }
}

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2) {
        if(HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin) == GPIO_PIN_RESET) {
            pb_state = 0;
        } else {
            pb_state = 1;
            pb_toggle ^= pb_state;
            HAL_GPIO_WritePin(LD1_GPIO_Port,LD1_Pin,GPIO_PIN_RESET);
        }

        HAL_TIM_Base_Stop_IT(&htim2); // Stop debounce timer
    }
}


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    int8_t bme280_rslt;
    uint8_t bme280_status_reg;
    uint32_t bme280_period;
    struct bme280_dev bme280;
    struct bme280_settings bme280Settings;
    struct bme280_data bme280_comp_data;

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_USART2_UART_Init();
    MX_UART5_Init();
    MX_TIM2_Init();
    MX_I2C1_Init();

    /* Initialize printf UART redirect */
    RetargetInit(&huart3);

    // Initialize PMS5003 sensor
    sensor_init();

    TRACE_INFO("*******************************\r\n");
    TRACE_INFO("Welcome to air quality monitor.\r\n");
    TRACE_INFO("*******************************\r\n");

    // BME280 Initialization
    bme280.intf = BME280_I2C_INTF;
    bme280.write = bme280_write;
    bme280.read = bme280_read;
    bme280.delay_us = bme280_delay;

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
        bme280Settings.filter = BME280_FILTER_COEFF_2;

        /* Over-sampling rate for humidity, temperature and pressure */
        bme280Settings.osr_h = BME280_OVERSAMPLING_1X;
        bme280Settings.osr_p = BME280_OVERSAMPLING_1X;
        bme280Settings.osr_t = BME280_OVERSAMPLING_1X;

        /* Setting the standby time */
        bme280Settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

        bme280_rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &bme280Settings, &bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Set Sensor settings failed.\r\n");
            bme280_error_codes_print_result("bme280_set_sensor_settings", bme280_rslt);
            break;
        }

        /* Always set the power mode after setting the configuration */
        bme280_rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Set Sensor mode failed.\r\n");
            bme280_error_codes_print_result("bme280_set_sensor_mode", bme280_rslt);
            break;
        }

        /* Calculate measurement time in microseconds */
        bme280_rslt = bme280_cal_meas_delay(&bme280_period, &bme280Settings);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 calculate measurement time failed.\r\n");
            bme280_error_codes_print_result("bme280_cal_meas_delay", bme280_rslt);
            break;
        }

        TRACE_INFO("\nTemperature calculation (Data displayed are compensated values)\r\n");
        TRACE_INFO("Measurement time : %lu us\r\n", (long unsigned int)bme280_period);

    } while (0);


    /* Output clock speeds of the MCU */
    TRACE_DEBUG("HCLK=%lu Hz\r\n",HAL_RCC_GetHCLKFreq());
    TRACE_DEBUG("PCLK1=%lu Hz\r\n",HAL_RCC_GetPCLK1Freq());
    TRACE_DEBUG("PCLK2=%lu Hz\r\n",HAL_RCC_GetPCLK2Freq());
    TRACE_DEBUG("SYSCLK=%lu Hz\r\n",HAL_RCC_GetSysClockFreq());

    /* Infinite loop */
    TRACE_INFO("Running main loop...\r\n");
    ConsoleInit();

    while (1)
    {
        if (PMS_read(&PMS5003) == PMS_OK) {

          sprintf(mesg, "\r\n\
            PM1.0 factory	  is : %d \r\n\
            PM2.5 factory	  is : %d \r\n\
            PM10  factory	  is : %d \r\n\
            PM1.0 atmospheric is : %d \r\n\
            PM2.5 atmospheric is : %d \r\n\
            PM10  atmospheric is : %d \r\n\
            0.3um density	  is : %d \r\n\
            0.5um density	  is : %d \r\n\
            1.0um density 	  is : %d \r\n\
            2.5um density 	  is : %d \r\n\
            5.0um density 	  is : %d \r\n\
            10um  density	  is : %d \r\n", PMS5003.PM1_0_factory,
                              PMS5003.PM2_5_factory, PMS5003.PM10_factory, PMS5003.PM1_0_atmospheric,
                              PMS5003.PM2_5_atmospheric, PMS5003.PM10_atmospheric, PMS5003.density_0_3um,
                              PMS5003.density_0_5um, PMS5003.density_1_0um, PMS5003.density_2_5um,
                              PMS5003.density_5_0um, PMS5003.density_10um
          );

          TRACE_INFO("PMS5003: %s\r\n",mesg);
      }

        if(pb_toggle) {
            HAL_GPIO_WritePin(USER_Btn_GPIO_Port,USER_Btn_Pin,GPIO_PIN_RESET);
        } else {
            HAL_GPIO_TogglePin(LD1_GPIO_Port,LD1_Pin);
        }

        ConsoleProcess();

        // Get BME280 temperature
        bme280_rslt = bme280_get_regs(BME280_REG_STATUS, &bme280_status_reg, 1, &bme280);
        if(bme280_rslt < 0) {
            TRACE_INFO("BME280 Get regs failed.\r\n");
            break;
        }

        if (1 /*bme280_status_reg & BME280_STATUS_MEAS_DONE*/)
        {
            /* Measurement time delay given to read sample */
            bme280.delay_us(bme280_period, bme280.intf_ptr);

            /* Read compensated data */
            bme280_rslt = bme280_get_sensor_data(BME280_TEMP, &bme280_comp_data, &bme280);
            if(bme280_rslt < 0) {
                TRACE_INFO("BME280 Get sensor data failed.\r\n");
                break;
            }

            TRACE_INFO("BME280:\r\n   "
                   "Temperature: %.3f deg C\r\n", bme280_comp_data.temperature);
        }

        HAL_Delay(100);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        //Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{
    huart5.Instance = UART5;
    huart5.Init.BaudRate = 9600;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart5) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : USER_Btn_Pin */
    GPIO_InitStruct.Pin = USER_Btn_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

    // Make the User button IT
    // Give the User button lowest IT priority
    HAL_NVIC_SetPriority(EXTI15_10_IRQn,15,0);
    // Enable the IRQ
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
    GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}


/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 42;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 50000;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_TIMING;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM2_IRQn, 14, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

}

/*
 * @brief Function for PMS5003 configuration
 */
void sensor_init(void)
{
    PMS5003.PMS_huart = &huart5; 			// passing uart handler to communicate with sensor
    PMS5003.PMS_MODE = PMS_MODE_ACTIVE;	// choosing the MODE
    if (PMS_Init(&PMS5003) != PMS_OK)
    {
        TRACE_INFO("PMS5003 Sensor Initialization Failed.\r\n");
        Error_Handler();
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

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
        // IMPLEMENT ME!
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
