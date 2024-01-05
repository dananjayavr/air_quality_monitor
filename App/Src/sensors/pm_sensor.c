//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//
#include <string.h>
#include "main.h"
#include "pm_sensor.h"
#include "ssd1306.h"

PMS_typedef PMS5003 = {0};
char mesg[1000] = {0};

extern UART_HandleTypeDef huart5; // PMS5003 particulate matter sensor

extern sensor_data_t sensor_data_all;

/*
 * @brief Function for PMS5003 configuration
 */
void pm_sensor_init(void)
{
    PMS5003.PMS_huart = &huart5; 			// passing uart handler to communicate with sensor
    PMS5003.PMS_MODE = PMS_MODE_ACTIVE;	// choosing the MODE
    if (PMS_Init(&PMS5003) != PMS_OK)
    {
        TRACE_INFO("PMS5003 Sensor Initialization Failed.\r\n");
        Error_Handler();
    }
}

/*
 * @brief Function to read PMS5003 data
 */
void pm_sensor_read(void) {
    char buffer[32];
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

        sensor_data_all.PM10_atmospheric = PMS5003.PM10_atmospheric;
        sensor_data_all.PM1_0_atmospheric = PMS5003.PM1_0_atmospheric;
        sensor_data_all.PM2_5_atmospheric = PMS5003.PM2_5_atmospheric;

        // PM2.5 is used when describing pollutant levels both outdoor and indoor,
        // where health impact from exposure considers amount of PM2.5 over a 24-hour period.
        // Most studies indicate PM2.5 at or below 12 μg/m3 is considered healthy with little to no risk from exposure.
        // If the level goes to or above 35 μg/m3 during a 24-hour period,
        // the air is considered unhealthy and can cause issues for people with existing breathing issues such as asthma.
        // Prolonged exposure to levels above 50 μg/m3 can lead to serious health issues and premature mortality.
        // Source: https://www.indoorairhygiene.org/pm2-5-explained/

        // Only considering PM2.5
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("PM2.5", Font_7x10, White);
        ssd1306_SetCursor(0, 10);

        memset(buffer,0,32);

        if(PMS5003.PM2_5_atmospheric < 12) {
            sprintf(buffer, "Good (%d)", PMS5003.PM2_5_atmospheric);
            ssd1306_WriteString(buffer, Font_11x18, White);
        } else if (PMS5003.PM2_5_atmospheric >= 12 && PMS5003.PM2_5_atmospheric <= 35) {
            sprintf(buffer, "Fair (%d)", PMS5003.PM2_5_atmospheric);
            ssd1306_WriteString(buffer, Font_11x18, White);
        } else {
            sprintf(buffer, "Bad (%d)", PMS5003.PM2_5_atmospheric);
            ssd1306_WriteString(buffer, Font_11x18, White);
        }

        ssd1306_UpdateScreen();
        TRACE_DEBUG("PMS5003: %s\r\n",mesg);
    }
}
