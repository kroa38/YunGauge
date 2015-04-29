
#ifndef DRV324P_H
#define DRV324P_H

#include "ioavr.h"
#include "inavr.h"

#define INPUT_IO             0U
#define OUTPUT_IO            1U


typedef volatile struct Struct_Atmega_Int {
  volatile Int8U bSensor_Int_Flag:1U;
  volatile Int8U bRTC_Tic_Int_Flag:1U;
}Struct_Atmega_Int;


/***********************************************************************************************************************
 *                                                                                                                     *
 *                        MAPPING DES I/O   du µC                                                                      *
 *                                                                                                                     *
 *********************************************** PORT A ****************************************************************/



//  ------------- PORT A -------------------------

#define SENSOR_WATER_DIR             DDRA_DDA0
#define SENSOR_WATER_PORT            PORTA_PORTA0
#define SENSOR_WATER_PIN             PINA_PINA0
#define PCINT_SENSOR_WATER           PCINT0

#define SENSOR_DOOR_DIR             DDRA_DDA1
#define SENSOR_DOOR_PORT            PORTA_PORTA1
#define SENSOR_DOOR_PIN             PINA_PINA1
#define PCINT_SENSOR_DOOR           PCINT1

#define CTS_DIR                  DDRA_DDA6
#define CTS_PORT                 PORTA_PORTA6
#define CTS_PIN                  PINA_PINA6

#define RTS_DIR                  DDRA_DDA7
#define RTS_PORT                 PORTA_PORTA7
#define RTS_PIN                  PINA_PINA7


//  ------------- PORT B -------------------------

#define BUSY_AVR_DIR                 DDRB_DDB0             // Est aussi un interruption PCINT8  -> PCIE1
#define BUSY_AVR_PORT                PORTB_PORTB0
#define BUSY_AVR_PIN                 PINB_PINB0            // lecture KEY Read S2

#define LED_ROUGE_DIR                DDRB_DDB1
#define LED_ROUGE_PORT               PORTB_PORTB1
#define LED_ROUGE_PIN                PINB_PINB1

#define LED_VERTE_DIR                DDRB_DDB2
#define LED_VERTE_PORT               PORTB_PORTB2
#define LED_VERTE_PIN                PINB_PINB2

#define LED_ORANGE_DIR                DDRB_DDB3
#define LED_ORANGE_PORT               PORTB_PORTB3
#define LED_ORANGE_PIN                PINB_PINB3
                                       
//  ------------- PORT C -------------------------

#define SCL_DIR                     DDRC_DDC0
#define SCL_PORT                    PORTC_PORTC0
#define SCL_PIN                     PINC_PINC0
    
#define SDA_DIR                     DDRC_DDC1
#define SDA_PORT                    PORTC_PORTC1
#define SDA_PIN                     PINC_PINC1


//  ------------- PORT D -------------------------

#define TELEINFO_RAW_DIR              DDRD_DDD0
#define TELEINFO_RAW_PORT             PORTD_PORTD0
#define TELEINFO_RAW_PIN              PIND_PIND0

#define TXD_AVR_DIR                   DDRD_DDD1
#define TXD_AVR_PORT                  PORTD_PORTD1
#define TXD_AVR_PIN                   PIND_PIND1

#define RTC_TIC_DIR                 DDRD_DDD2
#define RTC_TIC_PORT                PORTD_PORTD2
#define RTC_TIC_PIN                 PIND_PIND2
#define PCINT_RTC_TIC               PCINT26

//  ------------- INTERRUPTION  PORT FUNCTION -------------------------

enum{
  INT_SENSOR_WATER,
  INT_SENSOR_DOOR,
  INT_RTC_TIC,
};
  


//  ------------- FONCTIONS -------------------------

void Drv324P_Set_IO(void);
void Drv324P_Send_Reg_To_RS232(Int8U DeviceAddr,Int8U Reg);
void Drv324p_Interrupt(Int8U mode,Int8U Interrupt_Name);
void Drv324p_Disable_Watchdog(void);
void Drv324p_Enable_Watchdog(void);
void Drv324p_Reset_By_Watchdog(void);
void Drv324p_I2C_RequestToSend(void);
void Drv324p_I2C_ClearToSend(void);
#pragma vector = PCINT3_vect
__interrupt void Drv7513_PCINT3_ISR(void);

#pragma vector = PCINT0_vect
__interrupt void Drv7842_PCINT0_ISR(void);


#endif
