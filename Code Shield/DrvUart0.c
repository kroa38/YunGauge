

#include "typedefs.h"
#include "DrvUart0.h"
#include "DrvTwi.h"
#include "Srv_Out.h"
#include "Drv1338.h"

/*******************************************************************************
*	Variables globales
*******************************************************************************/
Int8U Index_Buffer_Uart_Current_Line ;
Int8U Index_Buffer_Uart_Current_Char ;
char Uart0_Buffer[BUFFER_UART0_NB_LINE_MAX][BUFFER_UART0_NB_CHAR_MAX];
char Uart0_Buffer_Diff[DIFF_COUNTER_LENGTH];           
Struct_Uart0_Flag S_Uart0_Flag;        // flags pour les trames rx et tx.
extern Struct_Timeout_Flag S_Timeout_Flag;
extern Struct_Srv_Event S_Srv_Event;
/*******************************************************************************
*	void Drv_Uart0_Init_Uart(void)
Uart qui recoit les données brut de la teleinfo
1200 bauds , 7bits,parité paire , 1 stop
*******************************************************************************/

void Drv_Uart0_Init_Uart_Rx(void)
{

  
  UBRR0H = 0x01;                             // UART 0 à 1200 bauds
  UBRR0L = 0x9F;                             // UART 0 à 1200 bauds
  
  UCSR0C_UCSZ00=0U;                          // 7 bits
  UCSR0C_UCSZ01=1U;                          // 7 bits

  UCSR0C_UPM00=0U;                          // parity even
  UCSR0C_UPM01=1U;                          // parity even
    
  
  UCSR0B_RXEN0 = 1U;                         // Active receive
  UCSR0B_TXEN0 = 0U;                         // Désactive transmit
  UCSR0B_RXCIE0 = 1U;                        // RX Complete Interrupt enable

  Drv_Uart0_Clear_Buffer();                  // reset buffer et flag
 }
/*******************************************************************************
*	void Drv_Uart0_Init_Uart(void)
UART qui communique à 19200 baud avec l'arduino et en 8,n,1
*******************************************************************************/
void Drv_Uart0_Init_Uart_Tx(void)
{

  UBRR0H = 0x00;                             // UART 0 à 19200 bauds
  UBRR0L = 0x19;                             // UART 0 à 19200 bauds
  
  UCSR0C_UCSZ00=1U;                          // 8 bits
  UCSR0C_UCSZ01=1U;                          // 8 bits

  UCSR0C_UPM00=0U;                          // no parity
  UCSR0C_UPM01=0U;                          // no parity
    
  
  UCSR0B_RXEN0 = 0U;                         // Désactive receive
  UCSR0B_TXEN0 = 1U;                         // Active transmit
  UCSR0B_RXCIE0 = 0U;                        // RX Complete Interrupt disable

 }
/*******************************************************************************
*	void Drv_Uart0_Disable_Uart_Tx(void)
Coupe l'uart
*******************************************************************************/
void Drv_Uart0_Disable_Uart_Tx(void)
{
  UCSR0B_TXEN0 = 0U;                           // désactive le transmetteur RS-232
}
/*******************************************************************************
*	interrupt void Drv_Uart0_Interrupt(void)
c'est ici que l'on réceptionne les caractères de la téléinfo

*******************************************************************************/

#pragma vector = USART0_RX_vect
__interrupt void Drv_Uart0_Interrupt(void)
{
  static Int8U tmp;
  
  tmp=UDR0;                               // il faut passer par une variable obligatoirement sinon le test ne marche pas
    
  if(tmp == STX_CHAR)                     // test si début de message
  {
    S_Uart0_Flag.bRx0_Started = 1U;       // la reception commence
    S_Uart0_Flag.bRx0_Finish = 0U;   
 
    return;
  }
  else if( (tmp == ETX_CHAR)  &&   S_Uart0_Flag.bRx0_Started)             // test si fin de message
  {
    S_Uart0_Flag.bRx0_Started = 0U;       // rx terminé
    S_Uart0_Flag.bRx0_Finish = 1U;        // le traitement de la trame reçue peut être lancé.
    UCSR0B_RXCIE0 = 0U;                   // desactive IT de l'uart0  
    UCSR0B_RXEN0 = 0U;                    // desactive receive    
    return;
  }


  if( S_Uart0_Flag.bRx0_Started )      // la capture commence que si STX reçu et si pas de CR_CHAR 
     {

       if(tmp != CR_CHAR)              // on ignore le caractère Carrier Return
       {
          if(tmp == LF_CHAR) 
            {
                if(Index_Buffer_Uart_Current_Line != BUFFER_UART0_NB_LINE_MAX)
                {
                Index_Buffer_Uart_Current_Line++;       // saut de ligne
                Index_Buffer_Uart_Current_Char=0U;
                }
            }
          else if (Index_Buffer_Uart_Current_Char != BUFFER_UART0_NB_CHAR_MAX)
            {
                Uart0_Buffer[Index_Buffer_Uart_Current_Line][Index_Buffer_Uart_Current_Char] = tmp;
                Index_Buffer_Uart_Current_Char++;
            }
       }
       
     UCSR0B_RXCIE0 = 1U;                     // réactive IT de l'uart0   
     }


}

/*******************************************************************************
void Drv_Uart0_IsMessageReceive(void)

cette fct doit être scrutée en permanance dans la boucle main.
elle permet de savoir si un message à été envoyé par le
compteur
*******************************************************************************/
void Drv_Uart0_IsMessageReceive(void)
{
  if(S_Uart0_Flag.bRx0_Started && S_Timeout_Flag.bTimeout_uart_rx_timeout)
  {
    S_Uart0_Flag.bRx0_Started=0U;                  // reset flag
  }
}
/*******************************************************************************
*	void Drv_Uart0_Send_String (int8U *T)

Cette fonction envoie une chaine de caractère sur la ligne RS232.
On envoi egalement CR+LF en fin de ligne
On vide le buffer de reception avant l'envoie.
*******************************************************************************/
void Drv_Uart0_Send_String (char *T)
{ 

  while(*T!=0U)
    {
      if(UCSR0A_UDRE0 == 1U)
        UDR0 = (unsigned char)(*(T++));
    }

}

/*******************************************************************************
void Drv_Uart0_Clear_Buffer(void)

Permet de vider le buffer de réception et de remettre les
index à zéro.
*******************************************************************************/
void Drv_Uart0_Clear_Buffer(void)
{
  Index_Buffer_Uart_Current_Line = 0U;  // index sur 0.
  Index_Buffer_Uart_Current_Char = 0U;  // tab sur 0.
  S_Uart0_Flag.bRx0_Started = 0U;
  S_Uart0_Flag.bRx0_Finish = 0U;
  
  
   for (Int8U ligne=0U; ligne<BUFFER_UART0_NB_LINE_MAX; ligne++)
  {  
       for (Int8U charactere=0U; charactere<BUFFER_UART0_NB_CHAR_MAX; charactere++)
      {
         Uart0_Buffer[ligne][charactere]=0U;                    // clear rx buffer
      }
  }
}

/*******************************************************************************
*	void Drv_Uart0_Send_CR_LF (void)
*******************************************************************************/
void Drv_Uart0_Send_CR_LF (void)
{
    while (UCSR0A_UDRE0 == 0U);              // wait for empty transmit buffer
    UDR0 = 0x0D;                             // send carriage return
    while (UCSR0A_UDRE0 == 0U);              // wait for empty transmit buffer
    UDR0 = 0x0A;                             // send line feed

}
/*******************************************************************************
void Drv_Uart0_Send_Char(int8U uart_data)
*******************************************************************************/
void Drv_Uart0_Send_Char(Int8U uart_data)
{
    Int8U temp,middle; 
     
    //UCSR0B_RXCIE0 = 0U;                     // desactive IT de l'uart1

    temp = uart_data;
    while (UCSR0A_UDRE0 == 0U);             // wait for empty transmit buffer

    middle = (temp/10U);                    // first digit of char uart_data
    UDR0 = middle + 0x30;
    temp = (temp-(middle*10U));
    while (UCSR0A_UDRE0 == 0U);             // wait for empty transmit buffer

    middle = temp;                          // second digit of char uart_data
    UDR0 = middle + 0x30;

    //UCSR0B_RXCIE0 = 1U;                      // reactive IT de l'uart0
}
/*******************************************************************************
void Drv_Uart0_Send_Double(int8U uart_data)
*******************************************************************************/
void Drv_Uart0_Send_Double(Int16U data)    // exemple avec 3417!
{
    Int16U tmp1,tmp2,tmp3,tmp4; 
     
    UCSR0B_RXCIE0 = 0U;                     // desactive IT de l'uart1
    
    while (UCSR0A_UDRE0 == 0U);            // wait for empty transmit buffer
    tmp1 = (data/1000U);                    // tmp1 = 3
    UDR0 = (Int8U)tmp1 + 0x30;              // on envoie 3.
    
    while (UCSR0A_UDRE0 == 0U);             // wait for empty transmit buffer
    tmp2 = ((data-(tmp1*1000U)))/100;       // tmp2 = 4          
    UDR0 = (Int8U)tmp2 + 0x30;              // on envoie 4.
    
    
    while (UCSR0A_UDRE0 == 0U);             // wait for empty transmit buffer
    tmp3 = (data-(tmp1*1000+tmp2*100))/10;  // tmp3 = (3417-3400)/10=1
    UDR0 = (Int8U)tmp3 + 0x30;              // on envoie 1.
    
    while (UCSR0A_UDRE0 == 0U);             // wait for empty transmit buffer
    tmp4 = (data-(tmp1*1000+tmp2*100+tmp3*10));  // tmp4 = 7
    UDR0 = (Int8U)tmp4 + 0x30;              // on envoie 7.


    
    UCSR0B_RXCIE0 = 1U;                      // reactive IT de l'uart0
    
}
/*******************************************************************************
Int32U Drv_Uart0_Acsii_to_Int32(char *ptrstr)
Convertit une chaine ascii en entier de 32bits
entrée : pointeur sur la chaine (9 caractères max donnant un chiffre de 0 à 2^32)
sortie : int32 non signé.
*******************************************************************************/
Int32U Drv_Uart0_Acsii_to_Int32(char *ptrstr)
{
  
   Int32U TabUL=0;
   Int8U length=0;      // nombre de caracteres de la chaine
   
  // calcul le nomre de chiffres ascii
  for ( Int8U i=0; i<9; i++ )
  {
    if( *(ptrstr+i) != 0)
    {
      length++;
    }
  }
   
    if(length>0)
    TabUL += (Int32U)(*(ptrstr+length-1) - 0x30);
    if(length>1)
    TabUL += (Int32U)(*(ptrstr+length-2) - 0x30) * 10;
    if(length>2)
    TabUL += (Int32U)(*(ptrstr+length-3) - 0x30) * 100;
    if(length>3)
    TabUL += (Int32U)(*(ptrstr+length-4) - 0x30) * 1000;
    if(length>4)
    TabUL += (Int32U)(*(ptrstr+length-5) - 0x30) * 10000;
    if(length>5)
    TabUL += (Int32U)(*(ptrstr+length-6) - 0x30) * 100000;
    if(length>6)
    TabUL += (Int32U)(*(ptrstr+length-7) - 0x30) * 1000000;
    if(length>7)
    TabUL += (Int32U)(*(ptrstr+length-8) - 0x30) * 10000000;
    if(length>8)
    TabUL += (Int32U)(*(ptrstr+length-9) - 0x30) * 100000000;

    return(TabUL);
 
  
}
/*******************************************************************************
char *  Drv_Uart0_Int32_To_Ascii(Int32U data)
Convertit un entier de 32bits en chaine ascii
entrée : int32 non signé.
entrée : pointeur sur tableau de characteres
sortie : retourne dans Uart0_Buffer_Diff le chiffre sur 9 caracteres
*******************************************************************************/
void Drv_Uart0_Int32_To_Ascii(Int32U TabResult, char *tmpbuff)
{
  Int32U decimal=0, i=0, rem=0;
      
    for ( Int8U i=0; i<DIFF_COUNTER_LENGTH; i++ )
  {
    tmpbuff[i]=0;
  }

    while (i<DIFF_COUNTER_LENGTH)
    {
        rem = TabResult%10;
        TabResult/=10;
        decimal = rem+0x30;
        tmpbuff[DIFF_COUNTER_LENGTH-1-i]=(Int8U)(decimal);  
        ++i;
    }    
    
    
}
/*******************************************************************************
char *  Drv_Uart0_Shift_Tab(Int32U data)
elimine les zero d'une chaine ascii
entrée : pointeur sur la chaine.
exemple en entrée 0001526152   -> donne 1526152 (3 zero éliminés)
exemple en entrée 0000000000   -> donne 0 (8 zero éliminés)
*******************************************************************************/
void Drv_Uart0_Shift_Tab(char *tmpbuff)
{
  Int8U i=0, j=0,index=0;
  
    if (tmpbuff[i]==0x30)     // si charatère "0"
    {     
        for ( i=0; i<DIFF_COUNTER_LENGTH; i++ )
      { 
        if (tmpbuff[i]!=0x30)     // si charatère "0"
        {
          index=i-1;
          i=DIFF_COUNTER_LENGTH;
        }
        
      }  
    
      if(index!=0)          // si on a au moins un nb >0
      {
            for (  j=index; j<DIFF_COUNTER_LENGTH; j++ )
          { 
                tmpbuff[j-index] = tmpbuff[j+1];
          }  
        
            for ( j=DIFF_COUNTER_LENGTH-index; j<DIFF_COUNTER_LENGTH; j++ )
          { 
                tmpbuff[j] = 0;
          }  
      }
      else              // sinon on met "0" zero unique dans la chaine
      {
                for ( i=1; i<DIFF_COUNTER_LENGTH; i++ )
                  { 
                    tmpbuff[i]=0;
                  }
      }
    }
  
}
/*******************************************************************************************
Int8U Uart_Request_To_Send(void)

Ecrit dans la NVram la valeur 1 pour indiqué que le transmetteur est prêt
à transmettre qqu chose sur l'UART.
La fonction entre dans une boucle et attend de voir le bit relaché
pour en sortir.
Celui-ci doit être relaché par l'arduino en ecrivant un 0 dans le NVRAM

In : void
Out : Int8U
********************************************************************************************/
Int8U Uart_Request_To_Send(void)
{
  Int8U tmp = 1U;
    Int8U tos = 1U;
    // on ecrit à l'adresse NVRAM pour indiquer que l'on veut transmettre qqwu chose sur l'uART
    // le nb de repetition d'envoie est remis à zero
    Drv324p_I2C_RequestToSend();  
    DrvTwi_Write_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_NVRAM_REG_UART_RTS_TELEINFO,1U);
    DrvTwi_Write_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_NVRAM_REG_UART_REPEAT,0U);
    tos = DrvTwi_Read_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_NVRAM_REG_UART_RTS_TELEINFO);    
    Drv324p_I2C_ClearToSend();

    __watchdog_reset();
    
    while(tmp)
    {
      Drv324p_I2C_RequestToSend();
      tmp = DrvTwi_Read_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_NVRAM_REG_UART_RTS_TELEINFO);
      Drv324p_I2C_ClearToSend();
      __watchdog_reset();
    }
    
    return 1UL;
}