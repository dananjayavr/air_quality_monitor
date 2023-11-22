//
// Created by Dananjaya RAMANAYAKE on 22/11/2023.
//

#ifndef _CO2_SENSOR_H
#define _CO2_SENSOR_H

#include "sgp30.h"

void sgp30_init_sensor(void);
void sgp30_read_sensor(void);

#endif //_CO2_SENSOR_H
