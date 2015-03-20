#include <Bridge.h>
#include <Console.h>
#include <FileIO.h>
#include <HttpClient.h>
#include <Mailbox.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>

#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
/********************************************************************************
DEFINITIONS
********************************************************************************/
#define BUSYPIN 4                          // n° de la pin Busy
#define RTSPIN 6                           // n° de la pin RTS (output)
#define CTSPIN 5                           // n° de la pin CTS (INPUT)
#define LEDVERTE 13                        // LED verte pour test
#undef DEBUG                               // sortie console pour debug
#define HOUR_ADJUST_CHECK 50UL*60UL*1000UL    // interval check pour la maj de l'heure de internet (50 minutes)
#define HOUR_ADJUST 20                     // heure de la mise a jour de l'heure internet
#define WAITFORLININO 50                   // temps d'attente de démarrage de linino (mini 50s)
#define SAMPLING_TELEINFO 106                // periode d'échantillonnage de teleinfo 1min,5min, 10min, 15min, 20min, 30min, 60min,106=every day at 6 oclock
#define NVRAM_SAMPLING_ADDR 0              // Adresse offset Nvram du DS1338 pour la periode d'échantillonage
/********************************************************************************
VARIABLES GLOBALES
********************************************************************************/
String dataString="";
char nb=0;
char current_day,current_hour;
unsigned long previousMillis ;        // will store last time 
struct Evenements
{
     unsigned char Ntp_To_Rtc_Update            : 1;
     unsigned char PingGoogle   		: 1;
     unsigned char WaitForLinino		: 1;
     unsigned char PrintRtcDate	                : 1;
     unsigned char PrintLininoDate              : 1;
     unsigned char StoreEventTeleinfoToFile     : 1;
     unsigned char StoreEventDoorToFile         : 1;  
     unsigned char Email_Ip_Address      	: 1; 
};

Evenements Event = {0};
RTC_DS1338 RTC;
SoftwareSerial mySerial(10, 11); // RX, TX

/********************************************************************************
FONCTION SETUP
********************************************************************************/

void setup() {

  GeneralInit();
  
}
/********************************************************************************
FONCTION LOOP
********************************************************************************/
void loop() 
{

  nb=0;
  dataString = "";
  char MsgChar;
  
  while (mySerial.available())            // boucle si reception de caractères
  { 
   MsgChar=char(mySerial.read());
   
   if( (MsgChar>31) && (MsgChar<127))    // on ne prend pas en compte tous les caractères
   {
   dataString += MsgChar;
   nb+=1;
   }
  }
  
  if(nb)
  {
      //mySerial.end();
      #ifdef DEBUG
      Serial.print("Event Received..!  ");
      //Serial.print(nb,DEC);
      Serial.print(' ');
      Serial.println(dataString);
      #endif
      mySerial.flush();
     
      if(dataString.startsWith("DOOR"))
      {
        Event.StoreEventDoorToFile = 1;
      }
      else
      {
        Event.StoreEventTeleinfoToFile = 1;
      }
      
  }
  
  Srv_Out_Event();

    
}
/********************************************************************************
FONCTION GeneralInit()
********************************************************************************/
void GeneralInit() {
uint8_t retour=0;

  previousMillis=millis();        // on recupère le temps depuis le demarrage de la carte
  pinMode(BUSYPIN, OUTPUT);
  pinMode(LEDVERTE, OUTPUT);
  pinMode(CTSPIN, INPUT);
  pinMode(RTSPIN, OUTPUT);  
  digitalWrite(BUSYPIN, HIGH);        // BUSY = 1 la carte Shield est en pause.
  digitalWrite(LEDVERTE, LOW);        // led status event
  ClearToSend();                      // libère la ligne I2C
 
  #ifdef DEBUG
  Serial.begin(115200);                 // init UART pour debug via USB.
  while (!Serial);         // wait for serial port to connect. 
  #endif
  
  Wire.begin();            // init I2C
  Bridge.begin();          // init Bridge
  FileSystem.begin();      // init file system 
  mySerial.begin(19200);   // réception de la téléinfo à 19200 bauds
  
  RequestToSend();
  RTC.writeSqwPinMode(OFF);           // led off de la RTC au demarrage
  delay(20);
  RTC.writenvram(NVRAM_SAMPLING_ADDR,SAMPLING_TELEINFO);          // periode d'échantillonnage de la teleinfo (en minutes)
  delay(20); 
  
  #ifdef DEBUG 
  retour =  RTC.readnvram(0);
  Serial.print("Nvram = "); 
  Serial.println(retour);
  #endif
  
  ClearToSend();
  
  Event.WaitForLinino = 1;  // on attend que Linino soit demarré (~ 50 secondes)
    
}
/********************************************************************************
FONCTION ProcExec
********************************************************************************/
String ProcExec(String Comm, String Par) {
  String Res = "";
  Process p;
  p.begin(Comm);
  p.addParameter(Par);
  p.run();

  while (p.available()>0) {
    char c = p.read();
    Res += c;
  }
  return Res;
}
/********************************************************************************
String LininoGetTimeStamp() 
// This function return a string with the time stamp
********************************************************************************/

String LininoGetTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
  //             T for the time hh:mm:ss
  time.run();  // run the command

  // read the output of the command
  while (time.available() > 0) {
    char c = time.read();
    if (c != '\n')
      result += c;
  }

  return result;
}
/*******************************************************************************
*	void Drv324p_ClearToSend(void)
Libère la ligne RTS
*******************************************************************************/
void RequestToSend(void)
{
        
    if(digitalRead(CTSPIN))
    {
      while(digitalRead(CTSPIN));
      delay(300);      
    }
    
    digitalWrite(RTSPIN, HIGH);
    
}
/*******************************************************************************
*	void Drv324p_ClearToSend(void)
Libère la ligne RTS
*******************************************************************************/
void ClearToSend(void)
{
  
    digitalWrite(RTSPIN, LOW); 

}
/***********************************************************
void Blink_Led(unsigned char count)
Fait clignoter la led verte x fois.
Paramètre : count
************************************************************/
void Blink_Led(unsigned char count)
{
  
      for(unsigned char i=0;i<count;i++)
    {
      digitalWrite(LEDVERTE, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                    // wait for a second
      digitalWrite(LEDVERTE, LOW);    // turn the LED off by making the voltage LOW
      delay(100);
    }
}
