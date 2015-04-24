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
#define BUSYPIN 4                           // n° de la pin Busy
#define RTSPIN 6                            // n° de la pin RTS (output)
#define CTSPIN 5                            // n° de la pin CTS (INPUT)
#define LEDVERTE 13                         // LED verte pour test
#define DEBUG                               // sortie console pour debug
#define HOUR_ADJUST_CHECK 50UL*60UL*1000UL  // interval check pour la maj de l'heure de internet (50 minutes)
#define HOUR_ADJUST_CHECK_THIN 1UL*60UL*1000UL  // interval check pour la maj de l'heure de internet (1 minutes)
#define HOUR_ADJUST 20                      // heure de la mise a jour de l'heure internet
#define WAITFORLININO  60                    // temps d'attente de démarrage de linino (mini 50s)
#define SAMPLING_TELEINFO 106               // periode d'échantillonnage de teleinfo 1min,5min, 10min, 15min, 20min, 30min, 60min,106=every day at 6 oclock
#define NVRAM_SAMPLING_ADDR 0               // Adresse offset Nvram du DS1338 pour la periode d'échantillonage
#define DATE_STRING_SIZE 25
#define RX_BUFFER_SIZE 70
#define DATE_ISO8601 1
#define DATE_CUSTOM 2
/********************************************************************************
VARIABLES GLOBALES
********************************************************************************/
char date_array[DATE_STRING_SIZE]="";
String dataString= "";
uint8_t adjust_rtc=HOUR_ADJUST;
//char nb=0;
char current_day,current_hour;
unsigned long previousMillis ;        // will store last time 
struct Evenements
{
     unsigned char Ntp_To_Rtc_Update            : 1;
     unsigned char PingGoogle   		: 1;
     unsigned char StoreEventTeleinfoToFile     : 1;
     unsigned char StoreEventDoorToFile         : 1;  
     unsigned char Ntp_To_RTC_OK        	: 1; 
     unsigned char dummy_one	                : 1;
     unsigned char dummy_two	                : 1;  
     unsigned char dummy_tree	                : 1;     
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

  char nb=0;
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
  mySerial.flush();
  if(nb)
  {
      //mySerial.end();
      #ifdef DEBUG
      Serial.print(F("Event Received..!  "));
      //Serial.print(nb,DEC);
      Serial.print(' ');
      Serial.println(dataString);
      #endif
      mySerial.flush();
     
      if(dataString.startsWith(F("DOOR")))
      {
        Event.StoreEventDoorToFile = 1;
      }
      else
      {
        Event.StoreEventTeleinfoToFile = 1;
      }
      
      
  }
  //delay(2000);

  Srv_Out_Event();

    
}
/********************************************************************************
FONCTION GeneralInit()
********************************************************************************/
void GeneralInit() {
  
  previousMillis=millis();        // on recupère le temps depuis le demarrage de la carte
  pinMode(BUSYPIN, OUTPUT);
  pinMode(LEDVERTE, OUTPUT);
  pinMode(CTSPIN, INPUT);
  pinMode(RTSPIN, OUTPUT);  
  digitalWrite(BUSYPIN, HIGH);          // BUSY = 1 la carte Shield est en pause.
  digitalWrite(LEDVERTE, LOW);          // led status event
  ClearToSend();                        // libère la ligne I2C
 
  #ifdef DEBUG
  Serial.begin(115200);                // init UART pour debug via USB.
  while (!Serial);                     // wait for serial port to connect. 
  Serial.println(F("START DEBUG ...... "));
  #endif
  
  Wire.begin();                        // init I2C
  Bridge.begin();                      // init Bridge
  FileSystem.begin();                  // init file system 
  mySerial.begin(19200);               // réception de la téléinfo à 19200 bauds
  
  RequestToSend();
  RTC.writeSqwPinMode(OFF);           // led off de la RTC au demarrage
  delay(20); 
  ClearToSend();
  Event.Ntp_To_RTC_OK =0;             // on n'a pas encore mis à jour l'heure par internet
  WaitForLinino();                    // on attend que Linino soit demarré (~ 50 secondes)
    
}
/********************************************************************************
FONCTION ProcExec
Lance un process 
In : String  (nom du process)
In : Parametre du process
Out : String
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
  time.begin(F("date"));
  time.addParameter(F("+%D-%T"));  // parameters: D for the complete date mm/dd/yy
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
/***********************************************************
int run_python_script_config(char *str)
Lit dans le fichier config.json la valeur du paramètre passé
In : pointeur de chaine de caracteres
out : uint8_t qui represente la valeur lue dans le fichier json
************************************************************/
int run_python_script_config(char *str)
{
  String tmpstr="";
  uint8_t tmp;
  
  Process shell;
  #ifdef DEBUG 
  Serial.print(F("lecture fichier config.py "));
  Serial.println(str);
  Serial.flush();
  #endif
  shell.begin(F("/root/yungauge/scripts/python/config.py"));
  shell.addParameter(str);
  shell.run();
  while (shell.available())
    {
      tmpstr += (char)shell.read();
    }
  tmp = tmpstr.toInt();
  Serial.flush();
  return tmp;
}
/***********************************************************
void Srv_PrintRtcDate()
Print sur la console uart la date. (debug uniquement)
************************************************************/
void PrintRtcDate()
{
    #ifdef DEBUG
    RequestToSend();    
    DateTime now = RTC.now(); 
    ClearToSend();    
    
    Serial.print(F("RTC Date: "));
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    #endif

}
/***********************************************************
void *getdate()

renvoie la date au format ISO8601 ou CUSTOM
dans le tableau date_array
in : format
out : pointeur vers date_array (variable globale)
************************************************************/
char *getdate(char format)
{
  for(char i=0;i<WAITFORLININO;i++)
  {
    date_array[i] = 0;
  }
    
  RequestToSend();
  DateTime now = RTC.now();
  ClearToSend();
  
  if(format==DATE_ISO8601)
  {
    String str = String(now.year(), DEC); 
    str += '-';
    str += String(now.month(), DEC);
    str += '-';
    str += String(now.day(), DEC);  
    str += 'T';  
    str += String(now.hour(), DEC); 
    str += ':';
    str += String(now.minute(), DEC);
    str += ':'; 
    str += String(now.second(), DEC); 
    str += String("+0100");
    
    str.toCharArray(date_array,25);
    
    return date_array;
  }
  else if(format==DATE_CUSTOM)
  {
    String str = String(now.day(), DEC); 
    str += '/';
    str += String(now.month(), DEC);
    str += '/';
    str += String(now.year(), DEC);  
    str += ',';  
    str += String(now.hour(), DEC); 
    str += ':';
    str += String(now.minute(), DEC);
    
    str.toCharArray(date_array,25);
    
    return date_array;    
  }
}
/***********************************************************
void Srv_PrintLininoDate()
Print sur la console uart la date et l'heure de Linino.
************************************************************/
void PrintLininoDate()
{
    #ifdef DEBUG
    String DateL = "";
    String TimeL = "";
    
    DateL = ProcExec(F("date"),F("+%Y-%m-%d %T"));
    TimeL = ProcExec("date","+%T");    

    Serial.print(F("Linino Date: "));
    Serial.println(DateL);
    Serial.print("  ");    
    Serial.println(TimeL);
    #endif
}
/***********************************************************
void WaitForLinino()
attente de demarrage de Linino ~50s
************************************************************/
void WaitForLinino()
{
  uint8_t jsonvalue;

    
    #ifdef DEBUG
    Serial.println(F("Wait for Linino"));
    #endif
	
    for(int i=0;i<WAITFORLININO;i++)
    {
      Blink_Led(1);
      #ifdef DEBUG
      Serial.print(F("."));
      #endif
      delay(1000);
    }
    
    #ifdef DEBUG
    Serial.println(F(""));
    Serial.println(F("Linux is ready "));
    #endif
    
    RequestToSend();
    RTC.writeSqwPinMode(SquareWave1HZ);	                      // clignotement de la led toutes les secondes
    jsonvalue = run_python_script_config("sampling_interval");          // lecture du fichier config.json
    RTC.writenvram(NVRAM_SAMPLING_ADDR,jsonvalue);            // periode d'échantillonnage de la teleinfo (en minutes)  
    adjust_rtc =run_python_script_config("adjust_rtc");       // recupère l'heure à laquelle on ajuste la RTC avec internet
    #ifdef DEBUG 
    jsonvalue =  RTC.readnvram(NVRAM_SAMPLING_ADDR);          // affiche le contenu de la nvram à l'adresse 0
    Serial.print("Nvram Sampling Teleinfo every ");  
    Serial.print(jsonvalue);
    Serial.println(" minutes");
    Serial.print("Adjust RTC every day at ");
    Serial.print(adjust_rtc);
    Serial.println(" hour");    
    #endif
    
    digitalWrite(BUSYPIN, LOW);        // BUSY = 0 la carte Shield peut demarrer.
    ClearToSend();  

}
