
#include "typedefs.h"
#include "Scripts_1338.h"
#include "Drv1338.h"


/*******************************************************************************
*	SCRIPT D'INITIALISATION DE L'ADDS1338
Initialise le DS1338 avec sa sortie SQW à 1Hz
Cette sortie est gérée en interruption par l'atmel.
*******************************************************************************/
__flash const Int8U SCRIPT_INIT_DS1338[SIZEOF_SCRIPT_INIT_DS1338] =
{
//I2C_DEVICE_ADDR_DS1338, DS1338_CONTROL_REG, 0x90 	        // Init SQW OUTPUT à 1Hz
I2C_DEVICE_ADDR_DS1338, DS1338_CONTROL_REG, 0x20	        // Init SQW OUTPUT à OFF
};
