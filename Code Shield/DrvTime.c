

#include "DrvTime.h"
#include "Drv324p.h"
#include "Srv_Out.h"

extern Struct_Atmega_Int S_Interrupt_Atmega;
extern Struct_Srv_Event S_Srv_Event;

/*******************************************************************************
*	VARIABLES GLOBALES
*******************************************************************************/
Struct_Timeout_Flag S_Timeout_Flag;

volatile struct Struct_Timer{
  volatile unsigned long timer_8ms  ;
  volatile Int16U timer_timout_bt_cnx;
  volatile Int8U timer_uart;
  volatile Int8U timer_debounce;
  volatile Int8U timer_edid;
  volatile Int8U timer_stdi_500ms;
  volatile Int8U timer_twi_timeout;
  volatile Int8U timer_key_debounce;
 
}S_Timer;
 

/*******************************************************************************
*	FONCTIONS
*******************************************************************************/

void DrvTime_InitTimer(void)
{
  S_Timer.timer_8ms=0UL;
  S_Timer.timer_uart=0U;
  S_Timer.timer_debounce = 0U;
  S_Timer.timer_stdi_500ms = 0U;
  S_Timer.timer_twi_timeout = 0U;

  // reset de TimerEventEvent (variable globale)
  S_Timeout_Flag.bTimeout_2ms=0U;
  S_Timeout_Flag.bTimeout_uart_rx_timeout=0U;
  S_Timeout_Flag.bTimeoutDebounce=0U;
}


/*******************************************************************************/
void DrvTime_TimerSetTimerUart_Rx_TimeOut(Int8U time)
{
  ATOMIC( S_Timer.timer_uart = time; )
}
/*******************************************************************************/
Int8U TimerGetTimerUart(void)
{
  return S_Timer.timer_uart;
}
/*******************************************************************************/
void DrvTime_TimerSetTimer8ms(unsigned long time)
{
  ATOMIC( S_Timer.timer_8ms = time; )
}
/*******************************************************************************/
unsigned long DrvTime_TimerGetTimer8ms(void)
{
  return S_Timer.timer_8ms;
}
/*******************************************************************************/
void DrvTime_TimerSetTimerDebounce(Int8U time)
{
  ATOMIC( S_Timer.timer_debounce = time; )
}
/*******************************************************************************/
Int8U DrvTime_TimerGetTimer100ms(void)
{
  return S_Timer.timer_debounce;
}

/*******************************************************************************
/	IRQ handler  interrupt void DrvTime_timer1_ovf_it(void)

debounce du clavier.
Le debounce timer règle la période d'échantillonnage basé sur 8.192ms.
Le debounce count règle le temps mini d'appui sur la touche pour qu'elle soit 
prise en compte.
*******************************************************************************/
#pragma vector=TIMER1_OVF_vect				// Interrupt vector = ansync timer overflow
__interrupt void DrvTime_timer1_ovf_it(void)
{	
  
}
/*******************************************************************************
/	IRQ handler  interrupt void DrvTime_timer0_ovf_it(void)
*******************************************************************************/
#pragma vector=TIMER0_OVF_vect				// Interrupt vector = ansync timer overflow
__interrupt void DrvTime_timer0_ovf_it(void)
{		
  
#define TIMER_500MS     61UL        // 500/0.008192 = 61
#define TIMER_100MS     11UL  
#define TIMER_2000MS    244UL
#define TIMER_1000MS    122UL
#define TIMER_90S       10986UL
  
  
    if (S_Timer.timer_8ms > 0UL )
    {
      --S_Timer.timer_8ms;
    }
    
    if(++S_Timer.timer_uart > TIMER_500MS)           // TIME OUT 500ms POUR BUFFER RX de CDE BLUETOOTH
    {
      S_Timer.timer_uart = 0U;                  
      S_Timeout_Flag.bTimeout_uart=1U;
    }

    if(++S_Timer.timer_debounce > TIMER_500MS)
    {
      S_Timer.timer_debounce = 0U;
      S_Timeout_Flag.bTimeoutDebounce=1U;
    }

    
    



}

/*******************************************************************************
	void DrvTime_Timer0_ovf_enable(void)      in fact 8.192ms

        Timer IT = I/O clk / 2^8 / Prescaler
        Le fusible du prescaler ne doit pas etre activé ! (Clock à 8MHz)
*******************************************************************************/

void DrvTime_Timer0_ovf_enable(void) {

  TIMSK0 &= ~((1U<<TOIE0)|(1U<<OCIE0A)|(1U<<OCIE0B));      // clear bits TOIE0,OCIE0A,OCIE0B: disable TC0 interrupt
  TCNT0 = 0U;                                              // RESET COUNTER 0
  TIMSK0 |= (1U<<TOIE0);	                               // set 8-bit Timer/Counter0 Overflow Interrupt Enable
  TCCR0A = 0U;                                             // NORMALE OPERATION
  //TCCR0B = (1U<<CS00) | (1U<<CS01);                       // PRESCALER = 64        (2.048ms timer = Fclk/256/64 = 8e6/256/64)
  TCCR0B = (1U<<CS02) ;                                     // PRESCALER = 256        (2.048ms timer = Fclk/256/256 = 8e6/256/256)
}
/*******************************************************************************
	void DrvTime_Timer1_ovf_enable(void)          in fact 8.192ms !

        Timer IT = I/O clk / 2^16 / Prescaler
        Le fusible du prescaler ne doit pas etre activé ! (Clock à 8MHz)
*******************************************************************************/

void DrvTime_Timer1_ovf_enable(void) {

  TIMSK1 &= ~((1U<<TOIE1)|(1U<<OCIE1A)|(1U<<OCIE1B));   // clear bits TOIE0,OCIE0A,OCIE0B: disable TC0 interrupt
  TCNT1L = 0U;                                          // RESET COUNTER 1 L
  TCNT1H = 0U;                                          // RESET COUNTER 1 H
  TCCR1A = 0U;                                          // NORMALE OPERATION
  TCCR1B = (1U<<CS10) ;                                 // PRESCALER = 1        (8.192ms timer = Fclk/65536/1 = 8e6/65536/1)

}

/***********************************************************************************************************************
*	     void DrvTime_Wait_Millisecondes(unsigned long tmp)
*
*        timer de 8.192ms (valeur passée doit etre >8 et <4000)
*
***********************************************************************************************************************/

void DrvTime_Wait_Millisecondes(unsigned long tmp) {


  if (tmp < 9UL) {tmp = 9UL;}                        // valeur mini passée = 9
  if (tmp > 4000UL) {tmp = 4000UL;}                  // 4s valeur maxi de passage
  
  tmp = tmp*1000UL/8192UL;                            // le timer à une periode de 8.192ms

  DrvTime_TimerSetTimer8ms( tmp );

  while(DrvTime_TimerGetTimer8ms() > 0UL)
  { 
        __watchdog_reset() ;      
   
  }

}

/***********************************************************************************************************************
*	      void DrvTime_Wait_Secondes(Int8U tmp);
*
*             timer multiple de 1s
*
***********************************************************************************************************************/

void DrvTime_Wait_Secondes(Int8U tmp) {

  static Int8U it=0U;

  for( it = 0U; it <= tmp-1U; it++)
  {
 DrvTime_Wait_Millisecondes(1000UL);                   // WAIT 1000MS
   __watchdog_reset();                       // Reset Watchdog
  }

}
