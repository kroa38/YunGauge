
#ifndef DRVLED_H
#define DRVLED_H

#include "typedefs.h"
#include "Drv324p.h"



typedef struct Struct_Led_Flag 
{
  Int8U LedD9;
  Int8U LedD10;
  Int8U LedD11;
  Int8U LedD12;
}Struct_Led_Flag;


enum
{
LED_ROUGE=0U,
LED_VERTE,
LED_ORANGE,
LED_D12,
LED_ON,
LED_OFF,
LED_BLINK,
};

void DrvLed_Led_Off(Int8U);
void DrvLed_Led_On(Int8U);
void DrvLed_Led_Toggle(Int8U);

#endif
