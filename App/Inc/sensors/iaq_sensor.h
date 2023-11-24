//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#ifndef _IAQ_SENSOR_H
#define _IAQ_SENSOR_H

#include "bme68x.h"
#include "bme68x_defs.h"

void bme688_init_sensor(void);
void bme688_read_sensor(void);
void bme688_error_codes_print_result(const char api_name[], int8_t rslt);

void bme688_iaq_algo(void);

#endif //_IAQ_SENSOR_H
