//
// Created by Dananjaya RAMANAYAKE on 21/11/2023.
//

#ifndef _IAQ_SENSOR_BSEC_H
#define _IAQ_SENSOR_BSEC_H

#include <string.h>
#include "debug.h"
#include "bme688_port.h"
#include "bme68x.h"
#include "bme68x_defs.h"

void bme688_bsec_init_sensor(void);
void bme688_bsec_read_sensor(void);

#endif //_IAQ_SENSOR_BSEC_H
