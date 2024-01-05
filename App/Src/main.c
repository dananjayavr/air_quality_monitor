/*
 *
 * Sensor Addresses:
 *
 * 5x5 RGB : I2C Addr 0x74
 * BME688 : I2C Addr 0x76
 * BME280 : I2C Addr 0x77
 * SGP30 : I2C Addr 0x58
 * AT24C256 : I2C Addr 0xA0 (EEPROM)
 *
 */

#define TRACE_LEVEL TRACE_LEVEL_INFO
#define BSEC_ENABLED 1

#include "main.h"
#include "hardware_init.h"
#include "sensors/pm_sensor.h"
#include "sensors/env_sensor.h"
#include "sensors/co2_sensor.h"
#include "ssd1306.h"
#include "at24c256_eeprom.h"

#if BSEC_ENABLED == 1
#include "sensors/iaq_sensor_bsec.h"
#else
#include "sensors/iaq_sensor.h"
#endif

volatile uint8_t pb_state; // hold push button state
volatile uint8_t pb_toggle; // hold toggled push button state

UART_HandleTypeDef huart2; // UART console
UART_HandleTypeDef huart3; // printf redirect
UART_HandleTypeDef huart5; // PMS5003 particulate matter sensor

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

sensor_data_t sensor_data_all = {0};
calibration_t write_calibration = {0};
calibration_t read_calibration = {0};

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

    if(htim->Instance == TIM4) {
        HAL_NVIC_DisableIRQ(TIM4_IRQn);
#if BSEC_ENABLED == 1
        bme688_bsec_read_sensor();
#else
        bme688_read_sensor();
#endif
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
    }

    if(htim->Instance == TIM5) {
        HAL_NVIC_DisableIRQ(TIM5_IRQn);
#if SGP30_ENABLED == 1
        sgp30_read_sensor();
#endif
        HAL_NVIC_EnableIRQ(TIM5_IRQn);
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
    MX_TIM4_Init();
    MX_TIM5_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();

    /* Initialize printf UART redirect */
    RetargetInit(&huart3);

    // Boot screen
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Initializing...", Font_7x10, White);
    ssd1306_UpdateScreen();

    HAL_Delay(100);

    TRACE_INFO("**********************************\r\n");
    TRACE_INFO("*** Indoor Air Quality Monitor ***\r\n");
    TRACE_INFO("**********************************\r\n");

    TRACE_INFO("Initializing particulate matter sensor...\r\n");
    // Initialize PMS5003 sensor
    pm_sensor_init();

    // Initialize BME280 sensor
    TRACE_INFO("Initializing environmental data sensor...\r\n");
    bme280_init_sensor();

    // Initialize BME688 sensor
    TRACE_INFO("Initializing gas sensor...\r\n");
#if BSEC_ENABLED == 1
    bme688_bsec_init_sensor();
#else
    bme688_init_sensor();
#endif

#if SGP30_ENABLED == 1
    // Initialize SGP30 sensor
    TRACE_INFO("Initializing CO2/VOC sensor...\r\n");
    sgp30_init_sensor();
#endif

    HAL_Delay(1000);

    TRACE_INFO("Initializing UART console...\r\n");
    ConsoleInit();

    HAL_TIM_Base_Start_IT(&htim4);
    HAL_TIM_Base_Start_IT(&htim5);

    TRACE_INFO("Running state machine...\r\n");

    ssd1306_Fill(Black);

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

void printSensorData(sensor_data_t data) {
    printf("PM1.0 Atmospheric: %" PRIu16 "\r\n", data.PM1_0_atmospheric);
    printf("PM2.5 Atmospheric: %" PRIu16 "\r\n", data.PM2_5_atmospheric);
    printf("PM10 Atmospheric: %" PRIu16 "\r\n", data.PM10_atmospheric);
    printf("Pressure: %lf\r\n", data.pressure);
    printf("Temperature: %lf\r\n", data.temperature);
    printf("Humidity: %lf\r\n", data.humidity);
    printf("TVOC (ppb): %" PRIu16 "\r\n", data.tvoc_ppb);
    printf("CO2 EQ (ppm): %" PRIu16 "\r\n", data.co2_eq_ppm);
    printf("IAQ: %f\r\n", data.iaq);
    printf("Accuracy: %f\r\n", data.accuracy);
    printf("CO2 EQ: %f\r\n", data.co2_eq);
    printf("Breath VOC: %f\r\n", data.breath_voc);
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
