
#ifndef DRVSENSOR_H
#define DRVSENSOR_H

#define SENSOR_WATER_TOOGLE 1       // Compteur d'eau
#define SENSOR_DOOR_TOOGLE 2       // Compteur evenementiel

extern volatile Int8U Sensor_DOOR_State; 

void DrvSensor_Init(void);
Int8U DrvSensor_Read(void);

#endif
