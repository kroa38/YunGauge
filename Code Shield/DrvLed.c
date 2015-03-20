
#include "DrvLed.h"

/*******************************************************************************
*	Variables globales
*******************************************************************************/
Struct_Led_Flag S_Led_Flag = {0U};


/*******************************************************************************
*	void Switch_Off_Led(Int8U led)
*******************************************************************************/
void DrvLed_Led_Off(Int8U led) {

  switch (led){

          case LED_ROUGE:
          LED_ROUGE_PORT = 0U;
          S_Led_Flag.LedD9=LED_OFF;
          break;

          case LED_VERTE:
          LED_VERTE_PORT = 0U;
          S_Led_Flag.LedD10=LED_OFF;
          break;

          case LED_ORANGE:
          LED_ORANGE_PORT = 0U;
          S_Led_Flag.LedD11=LED_OFF;
          break;
         

          
         default:
            __no_operation();
         break;
  }

}


/*******************************************************************************
*	void Switch_On_Led(Int8U led)
*******************************************************************************/
void DrvLed_Led_On(Int8U led) {

  switch (led){

          case LED_ROUGE:
          LED_ROUGE_PORT = 1U;
          S_Led_Flag.LedD9=LED_ON;
           break;

          case LED_VERTE:
          LED_VERTE_PORT = 1U;
          S_Led_Flag.LedD10=LED_ON;
           break;

          case LED_ORANGE:
          LED_ORANGE_PORT = 1U;
          S_Led_Flag.LedD11=LED_ON;
           break;
           
           
         default:
                __no_operation();
         break;
  }

}

/*******************************************************************************
*	void Toggle_Led(void)
*******************************************************************************/
void DrvLed_Led_Toggle(Int8U led) {

    switch (led){

          case LED_ROUGE:
          LED_ROUGE_PORT = ~LED_ROUGE_PIN;
          S_Led_Flag.LedD9=LED_BLINK;
           break;

          case LED_VERTE:
          LED_VERTE_PORT = ~LED_VERTE_PIN;
          S_Led_Flag.LedD10=LED_BLINK;
           break;
           
          case LED_ORANGE:
          LED_ORANGE_PORT = ~LED_ORANGE_PIN;
          S_Led_Flag.LedD11=LED_BLINK;
           break;

           
         default:
                __no_operation();
         break;
  }
}
