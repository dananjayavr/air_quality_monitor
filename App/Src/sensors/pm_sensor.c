//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//
#include "main.h"
#include "sensors/pm_sensor.h"

PMS_typedef PMS5003 = {0};
char mesg[1000] = {0};

extern UART_HandleTypeDef huart5; // PMS5003 particulate matter sensor

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
}
