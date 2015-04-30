
#include "typedefs.h"
#include "Drv324p.h"
#include "DrvTime.h"
#include "DrvSensor.h"



/*******************************************************************************
*	Variables globales
*******************************************************************************/
  
volatile Struct_Atmega_Int S_Interrupt_Atmega={0U};   // on met a zero les flag d'interruption
Int8U ActivatedSensor=0;

/*******************************************************************************
*	void Drv324P_Set_IO(void)

Cette fonction permet d'initialiser le IO du micro en entrée ou sortie
*******************************************************************************/
void Drv324P_Set_IO()
{
  
 
//  ------------- PORT A -------------------------

SENSOR_WATER_DIR        = INPUT_IO;
SENSOR_WATER_PORT       = 0U;                       // tri state HiZ input

SENSOR_DOOR_DIR        = INPUT_IO;
SENSOR_DOOR_PORT       = 0U;                       // tri state HiZ input

CTS_DIR             = INPUT_IO;                    // 
CTS_PORT            = 0U;                           // tri state HiZ input

RTS_DIR             = OUTPUT_IO;                     // 
RTS_PORT            = 0U;                           // 

//  ------------- PORT B -------------------------

BUSY_AVR_DIR         = INPUT_IO;                 // Detection I2C Busy
BUSY_AVR_PORT        = 0U;                      // 

LED_ROUGE_DIR          = OUTPUT_IO;              
LED_ROUGE_PORT         = 0U;                     // LED HDMI OFF

LED_VERTE_DIR         = OUTPUT_IO;               
LED_VERTE_PORT        = 0U;                      // LED VGA OFF

LED_ORANGE_DIR         = OUTPUT_IO;               
LED_ORANGE_PORT        = 0U;                      // LED CVBS OFF


//PB[5..7] utilisées pour prog SPI et non configurées

//  ------------- PORT C -------------------------
/*
SCL_DIR             = OUTPUT_IO;
SCL_PORT            = 1U;

SDA_DIR             = OUTPUT_IO;
SDA_PORT            = 1U;
*/
//PB[2..5] utilisées pour prog JTAG et non configurées

//  ------------- PORT D -------------------------

TELEINFO_RAW_DIR     = INPUT_IO;
TELEINFO_RAW_PORT    = 0U;                      // Doit être à l'etat bas a l'init de la RS232 sinon ça alimente le module

TXD_AVR_DIR          = OUTPUT_IO;
TXD_AVR_PORT         = 0U;                      // Doit être à l'etat bas a l'init de la RS232 sinon ça alimente le module BT

RTC_TIC_DIR        = INPUT_IO;
RTC_TIC_PORT       = 0U;                        // entrée d'interruption INT0

}

/*******************************************************************************
void Drv324p_Interrupt(Int8U mode,Int8U Interrupt_Name)

Active ou desactive les interruptions 
Paramètre : Mode (SET, CLEAR)
Paramètre : Interrupt_Name (Nom de l'interruption)
*******************************************************************************/
void Drv324p_Interrupt(Int8U mode,Int8U Interrupt_Name)
{
 
  switch (Interrupt_Name){  
  
  case INT_SENSOR_WATER:
    
    if(mode==SET)
    {
    PCMSK0 |= (1U << PCINT_SENSOR_WATER) ;
    PCICR |= (1U << PCIE0);
    PCIFR |= (1U << PCIF0);
    }
    else if (mode==CLEAR)
    {
    PCMSK0 &= ~(1U << PCINT_SENSOR_WATER) ;
    //PCICR &= ~(1U << PCIE0);
    PCIFR |= (1U << PCIF0);
   } 
   
   break;

  case INT_SENSOR_DOOR:
    
    if(mode==SET)
    {
    PCMSK0 |= (1U << PCINT_SENSOR_DOOR) ;
    PCICR |= (1U << PCIE0);
    PCIFR |= (1U << PCIF0);
    }
    else if (mode==CLEAR)
    {
    PCMSK0 &= ~(1U << PCINT_SENSOR_DOOR) ;
    //PCICR &= ~(1U << PCIE0);
    PCIFR |= (1U << PCIF0);
   } 
   
   break;   
   
  case INT_RTC_TIC:
    
    if(mode==SET)
    {
    PCMSK3 |= (1U << PCINT_RTC_TIC) ;
    PCICR |= (1U << PCIE3);
    PCIFR |= (1U << PCIF3);
    }
    else if (mode==CLEAR)
    {
    PCMSK3 &= ~(1U << PCINT_RTC_TIC) ;
    PCICR &= ~(1U << PCIE3);
    PCIFR |= (1U << PCIF3);
   } 
   break; 
     
   default :
      __no_operation();
   break;
    
  }
}
/*******************************************************************************
#pragma vector = PCINT0_vect
__interrupt void Drv324p_PCINT0_ISR(void)
*******************************************************************************/
#pragma vector = PCINT0_vect
__interrupt void Drv324p_PCINT0_ISR(void)
{ 
    S_Interrupt_Atmega.bSensor_Int_Flag=1U;   
    ActivatedSensor=DrvSensor_Read();

}

/*******************************************************************************
#pragma vector = PCINT3_vect
__interrupt void Drv324p_PCINT3_ISR(void)
*******************************************************************************/
#pragma vector = PCINT3_vect
__interrupt void Drv324p_PCINT3_ISR(void)
{
    if(RTC_TIC_PIN)             // on prend en compte que les niveau haut pour être à 1s 
  {
     S_Interrupt_Atmega.bRTC_Tic_Int_Flag=1U;   // Interruption RTC TIC
  }     
}


/***********************************************************************************************************************
*	void Drv324p_Enable_Watchdog(void)
************************************************************************************************************************/
void Drv324p_Enable_Watchdog(void)
{
Int8U em_isr_state;
em_isr_state = __save_interrupt();

__disable_interrupt();

__watchdog_reset();

/* Start timed sequence */
WDTCSR |= (1U<<WDCE) | (1U<<WDE);
/* Set new prescaler(time-out) (8s) */
WDTCSR = (1U<<WDE) | (1U<<WDP3) | (1U<<WDP0);        // 8s de watchdog

__restore_interrupt( em_isr_state );

}
/********************************************************************************************************************
*	void Drv324p_Disable_Watchdog(void)                                                                         *
*********************************************************************************************************************/

void Drv324p_Disable_Watchdog(void)
{
Int8U em_isr_state;
em_isr_state = __save_interrupt();

__disable_interrupt();                                                        // alors on les désactives

__watchdog_reset();

  MCUSR = 0x00;                       // clear all reset flag
  WDTCSR |= (1U<<WDCE) | (1U<<WDE) ;   // Enable a watchdog change
  WDTCSR = 0x00;                      // Disable watchdog

__restore_interrupt( em_isr_state );
}

/*******************************************************************************
*	void Drv324p_Reset_By_Watchdog(void)
*******************************************************************************/

void Drv324p_Reset_By_Watchdog(void)
{
  __disable_interrupt();

  WDTCSR |= (1U<<WDCE) | (1U<<WDE) ;                         // Enable a watchdog change
  WDTCSR = (1U<<WDE) ;                                      // Watchdog ON and Time Out = 16ms (pour un reset rapide !)
  while(1U);                                               // on boucle ce qui va provoquer un reset grace au watchdog

}

/*******************************************************************************
*	void Drv324p_I2C_RequestToSend(void)
Regarde la ligne CTS est à 1  pour voir si le port I2C est occupé.
Utilisée pour l'utilisation I2C en conjonction avec l'Arduino.
Met à 1 RTS pour prevenir le YUN que la ligne I2C est occupée
*******************************************************************************/
void Drv324p_I2C_RequestToSend(void)
{
    Int8U tmp;
    
    tmp = CTS_PIN  ;
    
    if(tmp)
    {
      while(tmp)
      {
      tmp = CTS_PIN;
      __delay_cycles(_50_MILLISECONDE);
      }      
    }
    
    RTS_PORT = 1;
    
}
/*******************************************************************************
*	void Drv324p_I2C_ClearToSend(void)
Libère la ligne RTS
*******************************************************************************/
void Drv324p_I2C_ClearToSend(void)
{
    __delay_cycles(_50_MILLISECONDE);
    RTS_PORT = 0; 
    

}
