

#include "typedefs.h"
#include "DrvSensor.h"
#include "DrvTime.h"
#include "Drv324p.h"

/*******************************************************************************
*	Variables globales
*******************************************************************************/
volatile Int8U Sensor_WATER_State=0U;
volatile Int8U Sensor_DOOR_State=0U;

/*******************************************************************************
/	void DrvSensor_Init(void)
*******************************************************************************/
void DrvSensor_Init(void)
{

Sensor_WATER_State = SENSOR_WATER_PIN;
Sensor_DOOR_State = SENSOR_DOOR_PIN;
  
}
/*******************************************************************************
/	Int8U DrvSensor_Read(void)
cette fonction renvoie le numéro du capteur qui à créer l'interruption
*******************************************************************************/
Int8U DrvSensor_Read(void)
{
  static Int8U tmp = 0U;
  static Int8U tmp1 = 0U;
  static Int8U tmp2 = 0U;
   
   tmp1 = Sensor_WATER_State;
   tmp2 = Sensor_DOOR_State;
   
  if(tmp1 != SENSOR_WATER_PIN)
  {
    tmp = SENSOR_WATER_TOOGLE;
    Sensor_WATER_State = SENSOR_WATER_PIN;
  }
  else if(tmp2 != SENSOR_DOOR_PIN)
  {
    tmp = SENSOR_DOOR_TOOGLE;
    Sensor_DOOR_State = SENSOR_DOOR_PIN;
  }
  return(tmp);
}
