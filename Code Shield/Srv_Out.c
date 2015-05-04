
#include "typedefs.h"
#include "Drv324p.h"
#include "Srv_Out.h"
#include "DrvLed.h"
#include "DrvTime.h"
#include "DrvTwi.h"
#include "ScriptMain.h"
#include "DrvUart0.h"
#include "DrvTime.h"
#include "DrvSensor.h"
#include "Drv1338.h"
#include <string.h>
#include <stddef.h>
#include <math.h>

Struct_Srv_Event S_Srv_Event = {0U};
Int16U Counter_RTC_TIC=0U;;  
extern Int8U ActivatedSensor;
extern Struct_Timeout_Flag S_Timeout_Flag;
extern Struct_Led_Flag S_Led_Flag;
extern Struct_Twi_Error_Flag S_Twi_Error_Flag;
extern Struct_Atmega_Int S_Interrupt_Atmega;
extern Struct_Uart0_Flag S_Uart0_Flag;                      // flags pour les trames rx et tx.
char *ptr_hchc,*ptr_hchp,*ptr_ptec;                         // pointeur vers le premier des caractères des 3 réponses, HC, HC TPEC
static char tab_diff_hp[DIFF_COUNTER_LENGTH+1];             // le +1 permet de rajouter le caractère 0 de fin de string
static char tab_diff_hc[DIFF_COUNTER_LENGTH+1];             // le +1 permet de rajouter le caractère 0 de fin de string
static Int8U tramefiltercount=TRAMEFILTERCOUNT;             // nb de fois que l'on recapture la trame suite à erreurs dans celle-ci
static Int32U previous_hc=0,previous_hp=0;
extern Int32U Index_HP_Test;
extern Int32U Index_HC_Test;
Int8U WaterCounter=0;

/*******************************************************************************************************
*	void Srv_Out_Event(void)

Cette fonction scrute toutes les évenements .

********************************************************************************************************/
void Srv_Out_Event(void)
{
  Srv_Out_Event_Sensor();                    // surveille evenement sur capteur
  Srv_Out_Event_RTC_TIC();                   // surveille si interruption de la RTC
  Srv_Out_Event_Filter_Teleinfo();           // filtre la trame venant de la capture teleinfo.
  Srv_Out_Event_Restart_Teleinfo_Capture();  // relance la capture teleinfo si il y a eu des erreurs dans la précédente capture
  Srv_Out_Event_Send_Teleinfo_to_Arduino();  // Envoie le code Teleinfo à l'Arduino  
  Srv_Out_Event_Send_Door_to_Arduino();      // Envoie le sensor 2 (sensor d'évenement)   
  

}


/*******************************************************************************************************
void Srv_Out_Event_ADV7513_Interrupt(void)
C'est ici que l'on lance la capture de la Teleinfo
toutes les 45 secondes.

rtc_nvram contient la valeur d'échantillonage de teleinfo qui peut être
toutes les 1min, 5min, 10min, 15min, 20min, 30min, 60min, ou tous les jours à heure fixe.
SI la Nvram contient 0 alors c'est une trame test qui est envoyée toutes les minutes.


********************************************************************************************************/
void Srv_Out_Event_RTC_TIC(void)
{
  Int8U rtc_nvram_sampling;
  

        if (S_Interrupt_Atmega.bRTC_Tic_Int_Flag)     // si interruption RTC TIC 
     {
           S_Interrupt_Atmega.bRTC_Tic_Int_Flag=0U;       
                      
           if(Counter_RTC_TIC++ >= COUNTER_TIC_SECONDE)
           {
                
                Drv324p_I2C_RequestToSend();
                rtc_nvram_sampling = DrvTwi_Read_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_NVRAM_REG_SAMPLING);   
                Drv324p_I2C_ClearToSend();
                
                if (rtc_nvram_sampling == 0U)                         // 0 = presence du mode test
                 {
                    teststring();                                     // teststring
                    Counter_RTC_TIC = 52U;                            // on entre toutes les 3 secondes en mode test
                    
                 }
                else if(Drv_DS1338_Synchro_With_RTC(rtc_nvram_sampling)) 
                 {
                    Counter_RTC_TIC = 0U;
                    Drv_Uart0_Init_Uart_Rx();                      // autorise la réception TELEINFO 
                    DrvLed_Led_On(LED_ORANGE);         
                    DrvTime_Wait_Millisecondes(100UL); 
                    DrvLed_Led_Off(LED_ORANGE);
                 }
               
           }          
     }

     
}

/*******************************************************************************************************
void Srv_Out_Event_ADV7513_Interrupt(void)
Gère les interruption CEC
********************************************************************************************************/
void Srv_Out_Event_Sensor(void)
{
  
    if (S_Interrupt_Atmega.bSensor_Int_Flag)     // si interruption RTC TIC 
     {
      Drv324p_Interrupt(CLEAR,INT_SENSOR_WATER);    // n'autorise pas IT sensor 1 
      Drv324p_Interrupt(CLEAR,INT_SENSOR_DOOR);    // n'autorise pas IT sensor 2
      
      S_Interrupt_Atmega.bSensor_Int_Flag=0U; // clear Flag
             
       if (ActivatedSensor == SENSOR_WATER_TOOGLE) 
       {
       WaterCounter++;
       }
       else if (ActivatedSensor == SENSOR_DOOR_TOOGLE) 
       {
        DrvTime_TimerSetTimerDebounce(0);
        S_Timeout_Flag.bTimeoutDebounce=0;
        S_Srv_Event.bDoorEvent=1;
       }
       
        Drv324p_Interrupt(SET,INT_SENSOR_WATER);    // autorise IT sensor 1

     }
}

/*******************************************************************************************************
void Srv_Out_Event_Filter_Teleinfo()


cette fonction va servir à filtrer la trame reçue et à determiner si il y a des erreur reçus 


La fct strstr sert à retrouver une chaine en retournant un pointeur sur le debut de celle-ci

On traite les 3 messages : 
HCHC  : Index Heures creuses (HCHC + 1 espace + 9 caractères + 1 espace + 1 CRC)  
HCHP  : Index Heures pleines (HCHP + 1 espace + 9 caractères + 1 espace + 1 CRC)
PTEC  : Période Tarifaire en cours (PTCE + 1 espace + 4 caractères + 1 espace + 1 CRC) 

Le message est envoyé sous la forme :
PTEC;HCHP;HCHC
********************************************************************************************************/
void Srv_Out_Event_Filter_Teleinfo(void)
{
  

  char sum=0;
  ptr_hchc = NULL;
  ptr_hchp = NULL;
  ptr_ptec = NULL;  
  
  
        if (S_Uart0_Flag.bRx0_Finish)                       // si interruption reception de trame complète
        { 
           S_Uart0_Flag.bRx0_Finish=0;
        
          // on recherche les chaines de caractères ici 
          // ------------------------------------------ 
               for ( Int8U ligne=0; ligne<BUFFER_UART0_NB_LINE_MAX; ligne++ )
               {
                 
                if( strstr(&(Uart0_Buffer[ligne][0]),"HCHC") ==  &(Uart0_Buffer[ligne][0]) )
                {
                  ptr_hchc = (&(Uart0_Buffer[ligne][0]));       // HCHC trouvé en debut de chaine
                }
                
               if( strstr(&(Uart0_Buffer[ligne][0]),"HCHP") ==  &(Uart0_Buffer[ligne][0]))
                {
                  ptr_hchp = (&(Uart0_Buffer[ligne][0]));       
                }
                
                
               if( strstr(&(Uart0_Buffer[ligne][0]),"PTEC") ==  &(Uart0_Buffer[ligne][0]) )
                {
                  ptr_ptec = (&(Uart0_Buffer[ligne][0]));       
                }              
                
              }
              
          // Controle du checksum de la chaine  
          // ------------------------------------------               
              
            if(ptr_hchc && ptr_hchp && ptr_ptec )          // si toutes les chaines sont trouvées alors on calcul le CRC
            {
               for ( Int8U i=0; i<LENGHT_HCHC; i++ )
               {
                 sum = (sum + *(ptr_hchc+i)) & 0x3F ;
               } 
               sum = sum + SP_CHAR;
               
               if(sum == *(ptr_hchc + LENGHT_HCHC +1))
               {
                 sum=0;                 
                 *(ptr_hchc + LENGHT_HCHC)=0;            // ajout du caractere de fin de chaine pour 
                 *(ptr_hchc + LENGHT_HCHC+1)=0;          // ajout du caractere de fin de chaine pour 
    
                   for ( Int8U i=0; i<LENGHT_HCHP; i++ )
                   {
                     sum = (sum + *(ptr_hchp+i)) & 0x3F ;
                   } 
                   sum = sum + SP_CHAR;
                   
                   if(sum == *(ptr_hchp + LENGHT_HCHP +1))
                   {   
                     sum=0;
                     *(ptr_hchp + LENGHT_HCHP)=0;
                     *(ptr_hchp + LENGHT_HCHP+1)=0;                     
        
                       for ( Int8U i=0; i<LENGHT_PTEC; i++ )
                       {
                         sum = (sum + *(ptr_ptec+i)) & 0x3F ;
                       } 
                       sum = sum + SP_CHAR;
                       
                       if(sum == *(ptr_ptec + LENGHT_PTEC +1))
                       {     
                        sum=0; 
                        *(ptr_ptec + LENGHT_PTEC-1)=0;      // rempli avec des zero 
                        *(ptr_ptec + LENGHT_PTEC-2)=0;
                        *(ptr_ptec + LENGHT_PTEC)=0; 
                        *(ptr_ptec + LENGHT_PTEC+1)=0;                        
                        S_Srv_Event.bTrameFilteredOk=1;
                        S_Srv_Event.bTrameFilteredNotOk=0;  
                        tramefiltercount=0;
                        
                        /*
                        // calcul la difference par rapport à la mesure précédente de HP 
                        Calc_Diff(&previous_hp,ptr_hchp+5,tab_diff_hp);
                        Drv_Uart0_Shift_Tab(tab_diff_hp);       // elimine les zero de l'entête de la chaine
                        // calcul la difference par rapport à la mesure précédente de HC 
                        Calc_Diff(&previous_hc,ptr_hchc+5,tab_diff_hc);
                        Drv_Uart0_Shift_Tab(tab_diff_hc);       // elimine les zero de l'entête de la chaine
                        
                        DrvLed_Led_On(LED_ROUGE);         
                        DrvTime_Wait_Millisecondes(500UL); 
                        DrvLed_Led_Off(LED_ROUGE);
                        */
                       }
                   }             
               }
            }
            else
            {
                         S_Srv_Event.bTrameFilteredOk=0;
                         S_Srv_Event.bTrameFilteredNotOk=1;
            }
          
       }
}
/************************************************************************
void Srv_Out_Event_Restart_Teleinfo_Capture(void)

cette fonction permet de relancer la capture lorsqu'une trame 
à été receptionnée avec des erreurs.
On autorise le max de relance avec la valeur TRAMEFILTERCOUNT.
***********************************************************************/
void Srv_Out_Event_Restart_Teleinfo_Capture(void)
{

     if (S_Srv_Event.bTrameFilteredNotOk)                       // si interruption reception de trame complète
        {  
          S_Srv_Event.bTrameFilteredNotOk=0;   
          
          tramefiltercount++;
          if(tramefiltercount<=TRAMEFILTERCOUNT)
          {
            Drv_Uart0_Init_Uart_Rx();                 // trame mauvaise alors on relance la capture
            DrvLed_Led_On(LED_ORANGE);         
            DrvTime_Wait_Millisecondes(50UL); 
            DrvLed_Led_Off(LED_ORANGE);            
          }
          else
          {
            tramefiltercount=0;
            UCSR0B_RXCIE0 = 0U;                   // desactive IT de l'uart0  
            UCSR0B_RXEN0 = 0U;                    // desactive receive uart 0            
          }
          
        }
}

/************************************************************************
void Srv_Out_Event_Send_Teleinfo_2_Arduino(void)

Envoie à l'Arduino en Uart les données Teleinfo.
HP + HC + PTEC + WaterCounteur

***********************************************************************/
void Srv_Out_Event_Send_Teleinfo_to_Arduino(void)
{
#define OUTPUT_BUFFER_SIZE 26  
  

  
   if (S_Srv_Event.bTrameFilteredOk)                       // si interruption reception de trame complète
        { 

          char OutputBuffer[OUTPUT_BUFFER_SIZE];
          char sum[2]={0,0};
          S_Srv_Event.bTrameFilteredOk=0;      
          
          for ( Int8U i=0; i<OUTPUT_BUFFER_SIZE; i++ )
          {
            OutputBuffer[i]=0;
          }
          
          strcpy(OutputBuffer,(ptr_hchp+5));            // ajout de la valeur HP ex 001524512 (9 caractères)
          strcat(OutputBuffer,",");                     // ajout virgule de séparation des champs CSV.
          strcat(OutputBuffer,(ptr_hchc+5));            // ajout on renvoi uniquement la valeur HC ex 007524415 (9 caractères)
          strcat(OutputBuffer,",");                     // ajout virgule de séparation des champs CSV. 
          strcat(OutputBuffer,(ptr_ptec+5));            // ajout on renvoi uniquement HP ou HC (2 caractères)
          strcat(OutputBuffer,",");                     // ajout virgule de séparation des champs CSV.
          
          for ( Int8U i=0; i<OUTPUT_BUFFER_SIZE; i++ )           // calcul CRC
            {
              sum[0] = (sum[0] + OutputBuffer[i]) & 0x3F ;
            }
          sum[0] = sum[0] + SP_CHAR;
            
          strcat(OutputBuffer,sum);             // ajout du CRC en fin de chaine.  
            
          if(Uart_Request_To_Send())
          { 
              Drv_Uart0_Init_Uart_Tx();                      // active le transmetteur RS-232            
              Drv_Uart0_Send_String(OutputBuffer);
              Drv_Uart0_Disable_Uart_Tx();                  // désactive le transmetteur RS-232 
          }
          
          DrvLed_Led_On(LED_VERTE);
          DrvTime_Wait_Millisecondes(200UL);
          DrvLed_Led_Off(LED_VERTE);   
          
            __no_operation();
            WaterCounter=0U;                             // reinitialise le compteur d'eau
        }
  
  
}
/************************************************************************
void Srv_Out_Event_Send_Door_to_Arduino(void)

envoie l'etat du sensor 2 à l'arduino.
l'etat de ce sensor est different du sensor 1 qui compte le nb d'impulsions pour
un compteur d'eau. Celui-rend compte immédiatement d'un evenement
il y a un debounce de 500ms avec un timer.

***********************************************************************/
void Srv_Out_Event_Send_Door_to_Arduino(void)
{
  
   if (S_Srv_Event.bDoorEvent && S_Timeout_Flag.bTimeoutDebounce) // si interruption reception de trame complète
        { 
            S_Srv_Event.bDoorEvent=0;

                                  // active le transmetteur RS-232 
            Sensor_DOOR_State = SENSOR_DOOR_PIN;           // mesure de l'état du capteur
            if(Sensor_DOOR_State == 1)
            {
                if(Uart_Request_To_Send())
                {
                    Drv_Uart0_Init_Uart_Tx();
                    Drv_Uart0_Send_String("DOOR CLOSE");
                    Drv_Uart0_Disable_Uart_Tx();                             // désactive le transmetteur RS-232
                }
                DrvLed_Led_On(LED_VERTE);
            }

            else
            {
                if(Uart_Request_To_Send())
                {              
                     Drv_Uart0_Init_Uart_Tx();  
                     Drv_Uart0_Send_String("DOOR OPEN");
                     Drv_Uart0_Disable_Uart_Tx();                             // désactive le transmetteur RS-232
                }
               DrvLed_Led_Off(LED_VERTE); 
            }
            
             Drv324p_Interrupt(SET,INT_SENSOR_DOOR);    // autorise IT Door
       }
}
 

/************************************************************************
void Calc_Diff(void)
calcul la difference entre l'index actuel et le précédent.
cette fonction est appelée par Srv_Out_Event_Filter_Teleinfo.

entrée 1 : pointeur sur l'index précédent
entrée 2 : pointeur sur l'index en cours
entrée 3 : pointeur sur la chaine de caractère de sortie
***********************************************************************/
void Calc_Diff(Int32U *previous_index, char *ptr_index, char *string_out)
{
  Int32U tmpdiff=0;
  
           if(*previous_index)                               // envoie le delta HP par rapport à la précédente mesure
            {                         
               tmpdiff = Drv_Uart0_Acsii_to_Int32(ptr_index)- *previous_index;    // calcul la difference.
               Drv_Uart0_Int32_To_Ascii(tmpdiff,string_out);                        // convertit en string
               *previous_index += tmpdiff;                                          // recupère le dernier index
            } 
          else
            {
              *previous_index = Drv_Uart0_Acsii_to_Int32(ptr_index);
              Drv_Uart0_Int32_To_Ascii(0UL,string_out);     // on met des caractères zero dans la chaine.
            }        
}
/*******************************************************************************************
void testcalc(void)
********************************************************************************************/
void testcalc(void)
{
  
  static char hp[] = "HCHP 004148963" ;
  static char hc[] = "HCHC 000524114" ;
  
  previous_hp = 0;
  previous_hc = 0;
  
  ptr_hchp = hp;
  ptr_hchc = hc;

  
  Calc_Diff(&previous_hp,ptr_hchp+5,tab_diff_hp);
  Drv_Uart0_Shift_Tab(tab_diff_hp);
  
  Calc_Diff(&previous_hc,ptr_hchc+5,tab_diff_hc);
  Drv_Uart0_Shift_Tab(tab_diff_hc);
  S_Srv_Event.bTrameFilteredOk=1;
  
 Srv_Out_Event_Send_Teleinfo_to_Arduino(); 
}
/*******************************************************************************************
void testcalc_shift(void)
********************************************************************************************/
void testcalc_shift(void)
{
  
  static char hp[] = "301568963" ;
 
  Drv_Uart0_Shift_Tab(hp);
  

}
/*******************************************************************************************
void testcalc_shift(void)
********************************************************************************************/
void testcat(void)
{
  #define TEST_CAT_BUFFER 35 
  
  char OutputBuffer[TEST_CAT_BUFFER];
  
  static char hp[] = "HCHP 004148963" ;
  static char hc[] = "HCHC 000524114" ;

  strcpy(OutputBuffer,(hp+5));            // ajout de la valeur HP ex 001524512 (9 caractères)
  strcat(OutputBuffer,",");                     // ajout virgule de séparation des champs CSV.
  strcat(OutputBuffer,(hc+5));            // ajout de la valeur HP ex 001524512 (9 caractères)
  strcat(OutputBuffer,",");                     // ajout virgule de séparation des champs CSV.
  
  asm("nop");
  
}
/*******************************************************************************************
void teststring(void)
Renvoie sur l'UART une chaine de charactères du type : 001526156,001287356,HP
le premier index est incrémenté à chaque passage dans la fct.
le second index ne bouge pas.
le mode est figé sur HP.
In : void
Out : void
********************************************************************************************/
void teststring(void)
{
#define TEST_STRING_BUFFER 26
  

  char OutputBuffer[TEST_STRING_BUFFER];
  char sum[2]={0,0};
  
  for ( Int8U i=0; i<TEST_STRING_BUFFER; i++ )
      {
        OutputBuffer[i]=0;
      }
  
  char tabhp[10];
  char tabhc[10];
  
  Index_HP_Test += 4UL;     
  
  Drv_Uart0_Int32_To_Ascii(Index_HP_Test,tabhp);
  Drv_Uart0_Int32_To_Ascii(Index_HC_Test,tabhc);
  
  strcpy(OutputBuffer,tabhp);            // ajout de la valeur HP ex 001524512 (9 caractères)
  strcat(OutputBuffer,",");              // ajout virgule de séparation des champs CSV.
  strcat(OutputBuffer,tabhc);            // ajout de la valeur HP ex 001524512 (9 caractères)
  strcat(OutputBuffer,",HP,");           // ajout virgule de séparation des champs CSV. 
  
  for ( Int8U i=0; i<TEST_STRING_BUFFER; i++ )           // calcul CRC
      {
        sum[0] = (sum[0] + OutputBuffer[i]) & 0x3F ;
      }
  sum[0] = sum[0] + SP_CHAR;
  
  strcat(OutputBuffer,sum);             // ajout du CRC en fin de chaine.
  
  
  asm("nop");

  
  if(Uart_Request_To_Send())
    { 
      Drv_Uart0_Init_Uart_Tx();             // active le transmetteur RS-232    
      Drv_Uart0_Send_String(OutputBuffer);             
      Drv_Uart0_Disable_Uart_Tx();          // désactive le transmetteur RS-232
    }  
  
  DrvLed_Led_On(LED_VERTE);         
  DrvTime_Wait_Millisecondes(100UL); 
  DrvLed_Led_Off(LED_VERTE);
 
  asm("nop");
  
}

