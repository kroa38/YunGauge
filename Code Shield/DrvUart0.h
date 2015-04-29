
#ifndef DRVUART0_H
#define DRVUART0_H
#include "Drv324p.h"


#define BUFFER_UART0_NB_LINE_MAX     15U            // nb de caracteres par lignes de message
#define BUFFER_UART0_NB_CHAR_MAX     25U            // nombre de lignes maxi
#define DIFF_COUNTER_LENGTH          9U             // longeur ascii du compteur de difference d'index 9 caractères 

/*

Une trame est constituée de trois parties.
•  le caractère "Start Text" STX (002 h) indique le début de la trame.
•  le corps de la trame est composé de plusieurs groupes d'informations.
•  le caractère "End Text" ETX (003 h) indique la fin de la trame.

La "checksum" est calculée sur l'ensemble des caractères allant du début du champ étiquette à la fin du champ
donnée, caractère SP inclus. On fait tout d'abord la somme des codes ASCII de tous ces caractères. Pour éviter
d'introduire des fonctions ASCII (00 à 1F en hexadécimal), on ne conserve que les six bits de poids faible du
résultat obtenu (cette opération se traduit par un ET logique entre la somme précédemment calculée et 03Fh).
Enfin, on ajoute 20 en hexadécimal. Le résultat sera donc toujours un caractère ASCII imprimable (signe, chiffre,
lettre majuscule) allant de 20 à 5F en hexadécimal.

exemple :

PAPP 00350 donne la somme de 50 41 50 50 20 30 30 33 35 30 = 0x249 
0x249 & 0x3F = 0x09
0x09+ 0x20 = 0x29    ---> code CRC

le CRC est le caractère après l'espace (0x20) des datas.


*/

#define STX_CHAR        0x02                    // start text
#define ETX_CHAR        0x03                    // stop text
#define EOT_CHAR        0x04                    // end of text
#define SP_CHAR         0x20                    // space
#define CR_CHAR         0x0D                    // carriage return ( fin du groupe d'information )
#define LF_CHAR         0x0A                    // line feed    (début du groupe)

typedef struct Struct_Uart0_Flag {
	Int8U bRx0_Started:1U ;                       // Flag pour indiquer que la réception commence
    Int8U bRx0_Finish:1U ;                       // Flag pour indiquer que la trame RX est complete
}Struct_Uart0_Flag;

extern Int8U Index_Buffer_Uart_Current_Line ;
extern Int8U Index_Buffer_Uart_Current_Char ;
extern char Uart0_Buffer[BUFFER_UART0_NB_LINE_MAX][BUFFER_UART0_NB_CHAR_MAX];

void Drv_Uart0_Init_Uart_Rx(void);
void Drv_Uart0_Init_Uart_Tx(void);
void Drv_Uart0_Send_CR_LF (void);
void Drv_Uart0_Send_String (char *T);
void Drv_Uart0_Decode_Frame(void);
void Drv_Uart0_IsMessageReceive(void);
void Drv_Uart0_Clear_Buffer(void);
void Drv_Uart0_Send_CR_LF (void);
void Drv_Uart0_Send_Double(Int16U data);
void Drv_Uart0_Send_Char(Int8U uart_data);
Int32U Drv_Uart0_Acsii_to_Int32(char *ptrstr);
void Drv_Uart0_Int32_To_Ascii(Int32U TabResult, char *tmpbuff);
void Drv_Uart0_Shift_Tab(char *tmpbuff);
void Drv_Uart0_Disable_Uart_Tx(void);
Int8U Uart_Request_To_Send(void);

#pragma vector=USART0_RX_vect
__interrupt void Drv_Uart0_Interrupt(void);


#endif


