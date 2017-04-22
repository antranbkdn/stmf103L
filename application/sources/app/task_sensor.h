#ifndef __TASK_SENSOR_H__
#define __TASK_SENSOR_H__
#include <stdint.h>

#include "../driver/SHT1x/SHT1x.h"
#include "../driver/thermistor/thermistor.h"
#include "../driver/hs1101/hs1101.h"
#include "../driver/EmonLib/EmonLib.h"


extern THERMISTOR thermistor;
extern hs1101_t hs1101;

extern EnergyMonitor ct_sensor1;
extern EnergyMonitor ct_sensor2;
extern EnergyMonitor ct_sensor3;
extern EnergyMonitor ct_sensor4;

#endif //__TASK_SENSOR_H__
