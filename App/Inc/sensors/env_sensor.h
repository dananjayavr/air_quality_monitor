//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#ifndef _ENV_SENSOR_H
#define _ENV_SENSOR_H

#include "main.h"

void bme280_init_sensor(void);
void bme280_read_sensor(void);
void bme280_error_codes_print_result(const char api_name[], int8_t rslt);

#endif //_ENV_SENSOR_H
