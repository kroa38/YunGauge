

#ifndef DRVTIME_H
#define DRVTIME_H

#include "typedefs.h"
#include "Drv324p.h"


/***************************************************************************************
*                                                                                      *
*                                TIMING                                                *
*                                                                                      *
****************************************************************************************/

/*********************** TIMING MILLISECONDE**********************************/
#define _1_MILLISECONDE         8000UL
#define _2_MILLISECONDE         16000UL
#define _10_MILLISECONDE        80000UL
#define _50_MILLISECONDE        400000UL
#define _100_MILLISECONDE       800000UL
#define _200_MILLISECONDE       2*800000UL
#define _300_MILLISECONDE       3*800000UL
#define _400_MILLISECONDE       4*800000UL
#define _500_MILLISECONDE       5*800000UL
#define _600_MILLISECONDE       6*800000UL
#define _700_MILLISECONDE       7*800000UL
#define _800_MILLISECONDE       8*800000UL
#define _900_MILLISECONDE       9*800000UL
#define _1000_MILLISECONDE      10*800000UL

#define _1_MICROSECONDE         8UL
#define _5_MICROSECONDE         40UL
#define _10_MICROSECONDE        80UL
#define _20_MICROSECONDE        160UL
#define _40_MICROSECONDE        320UL

typedef struct Struct_Timeout_Flag {
	Int8U bTimeout_2ms:1U;
	Int8U bTimeoutDebounce:1U;
    Int8U bTimeout_uart:1U;
    Int8U bTimeout_uart_rx_timeout:1U; 
}Struct_Timeout_Flag;


// fonction d'initialisation du module

void DrvTime_Timer0_ovf_enable(void);
void DrvTime_Timer1_ovf_enable(void);

void DrvTime_InitTimer(void);

void DrvTime_TimerSetTimer8ms(unsigned long time);
unsigned long DrvTime_TimerGetTimer8ms(void);


void TimerSetTimerDelay(long i_delay);
long TimerGetTimerDelay(void);

void DrvTime_TimerSetTimerDebounce(Int8U time);
Int8U DrvTime_TimerGetTimer100ms(void);


void DrvTime_Wait_Millisecondes(unsigned long tmp);
void DrvTime_Wait_Secondes(Int8U tmp);

void DrvTime_TimerSetTimerUart_Rx_TimeOut(Int8U time);
Int8U TimerGetTimerUart_bt(void);

#pragma vector=TIMER0_OVF_vect				// Interrupt vector = ansync timer overflow
__interrupt void DrvTime_timer0_ovf_it(void) ;

#pragma vector=TIMER1_OVF_vect				// Interrupt vector = ansync timer overflow
__interrupt void DrvTime_timer1_ovf_it(void) ;

#endif
