

#include "typedefs.h"
#include "DrvUart0.h"
#include "DrvTime.h"
#include "DrvLed.h"
#include "DrvTwi.h"
#include "Srv_Out.h"
#include "Drv324p.h"
#include "Drv1338.h"
#include "DrvSensor.h"

/*******************************************************************************
*	FONCTIONS
*******************************************************************************/
void General_Init(void);
void Wait_For_Yun(void);
void Need_Reset(void);
/*******************************************************************************
*	Variables globales
*******************************************************************************/

extern  Struct_Timeout_Flag S_Timeout_Flag;
extern  Struct_Atmega_Int S_Interrupt_Atmega;
extern  Struct_Srv_Event S_Srv_Event;
extern  Struct_Twi_Error_Flag S_Twi_Error_Flag;

/**********************************************************************************************************************
*                                                                                                                     *
*                                        __C_task void main(void)                                                     *
*                                                                                                                     *
**********************************************************************************************************************/

 __C_task void main(void) {


General_Init();                                   // init globale


          while( !S_Twi_Error_Flag.bi2c)
          {        
            Need_Reset();
            Srv_Out_Event();    
               __watchdog_reset();
          }

        while(1U)                           // on atterit ici lors d'une erreur I2C
        {
            S_Twi_Error_Flag.bDS1338S_ChipSet ? DrvLed_Led_On(LED_ROUGE):DrvLed_Led_Off(LED_ROUGE);        
            DrvTime_Wait_Millisecondes(500UL);
            DrvLed_Led_Off(LED_ROUGE);         
            DrvTime_Wait_Millisecondes(500UL); 
            DrvLed_Led_On(LED_ROUGE);
        }

 }


/***********************************************************************************************************************
*	void General_Init(void)    
*
* On initilise tous les p�riph�riques ici.
 Sauf le AD7842 ainsi que le ADV7513 qui sont initialis�s lors de la d�tection du mode video pr�sent.
 Les ADV7842 et ADV7513 sont en power_down d�s l'appel de la fonction Drv324P_Set_IO()
***********************************************************************************************************************/
void General_Init(void){    
    
  
          __disable_interrupt();
          Drv324p_Disable_Watchdog();               // d�sactive le watchdog au depart
          Drv324P_Set_IO();                         //Initilise les IO
          Wait_For_Yun();                           //attente demarrage carte yun 
          DrvLed_Led_On(LED_ROUGE);
          DrvLed_Led_On(LED_VERTE);
          DrvLed_Led_On(LED_ORANGE);   
          __delay_cycles(_300_MILLISECONDE);                 // attente demarrage carte Yun       
          DrvLed_Led_Off(LED_ROUGE);
          DrvLed_Led_Off(LED_VERTE);
          DrvLed_Led_Off(LED_ORANGE);  
          
          DrvTime_InitTimer();                      // init des variables timer
          __enable_interrupt();                     // positionner ici car les init suivantes utilise le timer qui est sur IT
          DrvTime_Timer0_ovf_enable();              // active le timer 0
          DrvTime_Timer1_ovf_enable();              // active le timer 1 (keyboard deboucing)
          MCUCR_PUD = 0;                            // Pull-UP on
          Drv324p_Enable_Watchdog();                // watchdog actif
          TWI_Master_Initialise();                  // enable I2C
          Drv324p_Interrupt(SET,INT_SENSOR_WATER);  // autorise pas la touche select  
          Drv324p_Interrupt(SET,INT_SENSOR_DOOR);   // autorise pas la touche select
          Drv324p_Interrupt(SET,INT_RTC_TIC);       // set interrupt from external RTC            
          DrvSensor_Init();
          //Drv_DS1338_Init();
          S_Interrupt_Atmega.bRTC_Tic_Int_Flag=0;
          asm("nop");

}
/***********************************************************************************************************************
*	void Wait_For_Yun(void)    
*
on attend ici que le Yun lib�re la ligne 4 qui informe qu'il est occup�

***********************************************************************************************************************/
void Wait_For_Yun(void)
{
  Int8U i=0;
  
  __watchdog_reset();
  while(i++<5)
  {
    __delay_cycles(_1000_MILLISECONDE);                 // attente demarrage carte Yun
    __watchdog_reset();
  }
  
  i=1;
  
  while(i)           // attend que le yun abaisse la ligne BUSY_AVR
  {
    i = BUSY_AVR_PIN;
  }
}
/***********************************************************************************************************************
*	void Need_Reset(void)   
*
Utilis� dans la boucle principale.
SI l'AVR du  Yun met � 1 la pin BUSY alors �ela fera entrer en reset l'AVR

***********************************************************************************************************************/
void Need_Reset(void)
{
   Int8U i=0;
   i = BUSY_AVR_PIN;
   
   if (i==1) Drv324p_Reset_By_Watchdog();
  
}