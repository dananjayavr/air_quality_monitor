#include "main.h"
#include "hardware_init.h"
#include "sensors/pm_sensor.h"
#include "sensors/env_sensor.h"
#include "sensors/iaq_sensor.h"

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

    TRACE_INFO("*******************************\r\n");
    TRACE_INFO("Welcome to air quality monitor.\r\n");
    TRACE_INFO("*******************************\r\n");

    TRACE_INFO("Initializing particulate matter sensor...\r\n");
    // Initialize PMS5003 sensor
    pm_sensor_init();


    // Initialize BME280 sensor
    TRACE_INFO("Initializing environmental data sensor...\r\n");
    bme280_init_sensor();


    // Initialize BME688 sensor
    //TRACE_INFO("Initializing indoor air quality sensor...\r\n");
    //bme688_init_sensor();

    HAL_Delay(1000);

    /* Output clock speeds of the MCU */
    TRACE_DEBUG("HCLK=%lu Hz\r\n",HAL_RCC_GetHCLKFreq());
    TRACE_DEBUG("PCLK1=%lu Hz\r\n",HAL_RCC_GetPCLK1Freq());
    TRACE_DEBUG("PCLK2=%lu Hz\r\n",HAL_RCC_GetPCLK2Freq());
    TRACE_DEBUG("SYSCLK=%lu Hz\r\n",HAL_RCC_GetSysClockFreq());

    TRACE_INFO("Initializing UART console...\r\n");
    ConsoleInit();

    TRACE_INFO("Running state machine...\r\n");
    /* Infinite loop */
    while (1)
    {
        pm_sensor_read();

        if(pb_toggle) {
            HAL_GPIO_WritePin(USER_Btn_GPIO_Port,USER_Btn_Pin,GPIO_PIN_RESET);
        } else {
            HAL_GPIO_TogglePin(LD1_GPIO_Port,LD1_Pin);
        }

        ConsoleProcess();

        bme280_read_sensor();

        //bme688_read_sensor();

        HAL_Delay(100);
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
