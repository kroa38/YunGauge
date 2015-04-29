
#include "DrvTwi.h"
#include "Drv1338.h"
#include "Drv324p.h"

/*******************************************************************************
*	Variables globales
*******************************************************************************/
Struct_Twi_Error_Flag S_Twi_Error_Flag = {0U};

/****************************************************************************
Call this function to set up the TWI master to its initial standby state.
Remember to enable interrupts from the main application after initializing the TWI.
****************************************************************************/
void TWI_Master_Initialise(void)
{
  Drv324p_I2C_RequestToSend();

  TWBR = TWI_TWBR;                                  // Set bit rate register (Baudrate). Defined in header file.
  TWSR = TWI_TWPS;                                  // Not used. Driver presumes prescaler to be 00.
  //USIDR = 0xFF;                                   // Default content = SDA released.
  TWCR = (1U<<TWEN)|                                // Enable TWI-interface and release TWI pins.
         (0U<<TWIE)|(0U<<TWINT)|                    // Disable Interupt.
         (0U<<TWEA)|(0U<<TWSTA)|(0U<<TWSTO)|        // No Signal requests.
         (0U<<TWWC);                                //
  
  Drv324p_I2C_ClearToSend();
}



/***********************************************************************************************************
*	Int8U DrvTwi_Read_Byte(Int8U Device_Addr,Int8U Word_Offset)
Cette focntion retourne la donnée I2C lue à l'adresse Word_Offset de Device_addr
************************************************************************************************************/
Int8U DrvTwi_Read_Byte(Int8U Device_Addr,Int8U Word_Offset)
{

static Int8U tmp;
        
          tmp=0U;

         __delay_cycles(_20_MICROSECONDE);
         

          TWCR = (1U<<TWINT)|(1U<<TWSTA)|(1U<<TWEN);           // SEND START
         while ( (!(TWCR & (1U<<TWINT))) );

          if (((TWSR==TWI_START) || (TWSR==TWI_REP_START)) )
          {
          TWDR =  Device_Addr & TWI_WRITE;                  // SEND SLAVE ADRESSE + WRITE
          TWCR = (1U<<TWINT) | (1U<<TWEN);
          while ( (!(TWCR & (1U<<TWINT))));

           if( (TWSR==TWI_MTX_ADR_ACK) )
           {
           TWDR = Word_Offset;                               //  SEND REG ADRRESS TO BE READ
           TWCR = (1U<<TWINT) | (1U<<TWEN);
           while ( (!(TWCR & (1U<<TWINT))) );

            if ( (TWSR==TWI_MTX_DATA_ACK) )
            {
            TWCR = (1U<<TWINT)|(1U<<TWSTA)|(1U<<TWEN);           // REPEAT START
            while ( (!(TWCR & (1U<<TWINT))) );

             if ( (TWSR==TWI_REP_START) )
             {
             TWDR =  Device_Addr | TWI_READ;                   // SEND SLAVE ADRESSE + READ
             TWCR = (1U<<TWINT) | (1U<<TWEN);
              while ( (!(TWCR & (1U<<TWINT))) );

              if ( (TWSR==TWI_MRX_ADR_ACK) )
              {
              TWCR = (1U<<TWINT) | (1U<<TWEN);                    // SEND CLOCK TO RECOVERY DATA
               while ( (!(TWCR & (1U<<TWINT))) );

               if ( ( TWSR==TWI_MRX_DATA_NACK) )
               {
               tmp= TWDR;                                       // STORE DATA
               TWCR = (1U<<TWINT)|(1U<<TWEN)| (1U<<TWSTO);          // SEND STOP

                if (TWSR==TWI_NO_STATE)
                {
                  __no_operation();
                }
                else
                {
                S_Twi_Error_Flag.bRead=1U;
                }
               }
                else
                {
                S_Twi_Error_Flag.bRead=1U;
                }
              }
                else
                {
                S_Twi_Error_Flag.bRead=1U;
                }
             }
                else
                {
                S_Twi_Error_Flag.bRead=1U;
                }
            }
                else
                {
                S_Twi_Error_Flag.bRead=1U;
                }
           }
                else
                {
                S_Twi_Error_Flag.bRead=1U;
                }
          }
                else
                {
                S_Twi_Error_Flag.bRead=1U;
                }


    if (S_Twi_Error_Flag.bRead)
    {   
    Twi_Device_Error_ID(Device_Addr);
    S_Twi_Error_Flag.bRead=0U;
    }

  
   return(tmp);
}

/***********************************************************************************************************
*	void DrvTwi_Write_Byte_2(Int8U Device_Addr,Int8U Word_Offset,Int8U Data)
************************************************************************************************************/
void DrvTwi_Write_Byte(Int8U Device_Addr,Int8U Word_Offset,Int8U Data)
{
           
           __delay_cycles(_20_MICROSECONDE);
           


          TWCR = (1U<<TWINT)|(1U<<TWSTA)|(1U<<TWEN);           // SEND START
          while ( (!(TWCR & (1U<<TWINT))) );

          if  ( ((TWSR==TWI_START) || (TWSR==TWI_REP_START)) )
          {
          TWDR =  Device_Addr & TWI_WRITE;                 // SEND SLAVE ADRESSE + WRITE
          TWCR = (1U<<TWINT) | (1U<<TWEN);
          while ( (!(TWCR & (1U<<TWINT))));

           if ( (TWSR==TWI_MTX_ADR_ACK) )
           {
           TWDR = Word_Offset;                               //  SEND REG ADRRESS TO BE WRITEN
           TWCR = (1U<<TWINT) | (1U<<TWEN);
           while ( (!(TWCR & (1U<<TWINT)))  );

            if ( (TWSR==TWI_MTX_DATA_ACK) )
              {
            TWDR =  Data;                                     // DATA TO WRITE
            TWCR = (1U<<TWINT) | (1U<<TWEN);
            while ( (!(TWCR & (1U<<TWINT))) );

             if ( (TWSR==TWI_MTX_DATA_ACK) )                  // --> erreur Bus Si on fait un reset de l'I2C du 7842
               {
             TWCR = (1U<<TWINT)|(1U<<TWEN)| (1U<<TWSTO);          // SEND STOP

                 if (TWSR==TWI_NO_STATE)
                 {
                   __no_operation();
                 }
                 else
                 {
                 S_Twi_Error_Flag.bWrite=1U;
                 }
               }
              else
              {

                   S_Twi_Error_Flag.bWrite=1U;
              }
              }
            else
            {
            S_Twi_Error_Flag.bWrite=1U;
            }
           }
           else
          {
          S_Twi_Error_Flag.bWrite=1U;
          }
         }
        else
          {
          S_Twi_Error_Flag.bWrite=1U;
          }

    
    if (S_Twi_Error_Flag.bWrite)
    {
    Twi_Device_Error_ID(Device_Addr);
    S_Twi_Error_Flag.bWrite=0U;
    }


}

/***********************************************************************************************************
*	Int8U Twi_Device_Error_ID(Int8U Device_Addr)
retourne le chip en default
************************************************************************************************************/
void Twi_Device_Error_ID(Int8U Device_Addr)
{
  
      S_Twi_Error_Flag.bi2c=1U;             // on indique qu'il y a bien une erreur I2C

      
          switch (Device_Addr)
      {  
      case  I2C_DEVICE_ADDR_DS1338:
      S_Twi_Error_Flag.bDS1338S_ChipSet=1U;
      break;
      }
    
}
