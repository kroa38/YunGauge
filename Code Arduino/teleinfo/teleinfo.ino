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
#define BUSYPIN 4                                 // n° de la pin Busy
#define RTSPIN 6                                  // n° de la pin RTS (output)
#define CTSPIN 5                                  // n° de la pin CTS (INPUT)
#define LEDVERTE 13                               // LED verte pour test
#define DEBUG                                     // sortie console pour debug
#define HOUR_ADJUST_CHECK 50UL*60UL*1000UL        // interval check pour la maj de l'heure de internet (50 minutes)
#define HOUR_ADJUST_CHECK_THIN 5UL*60UL*1000UL    // interval check pour la maj de l'heure de internet (1 minutes)
#define WAITFORLININO  1                         // temps d'attente de démarrage de linino (mini 50s)
#define DS1338_NVRAM_REG_SAMPLING              0  // Adresse offset Nvram du DS1338 pour la periode d'échantillonage
#define DS1338_NVRAM_REG_UART_RTS_TELEINFO     1  // RTS qui dit qu'un message teleinfo est reçu
#define DS1338_NVRAM_REG_UART_REPEAT           2  // demande de renvoie du message
#define DATE_STRING_SIZE 25
#define RX_BUFFER_SIZE 70
#define DATE_ISO8601 1
#define DATE_CUSTOM 2
#define UNIX_TIME 3
#define DOW 4
/********************************************************************************
VARIABLES GLOBALES
********************************************************************************/
char date_array[DATE_STRING_SIZE]="";
String dataString= "";
uint8_t adjust_rtc;
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
     unsigned char Uart_data_ready              : 1;
     unsigned char dummy_two	                : 1;  
     unsigned char dummy_tree	                : 1;     
};

Evenements Event = {0};
RTC_DS1338 RTC;
SoftwareSerial mySerial(10, 11); // RX, TX

/********************************************************************************
FONCTION SETUP
********************************************************************************/

void setup() 
{
  GeneralInit();
}
/********************************************************************************
FONCTION LOOP
********************************************************************************/
void loop() 
{
  /* gestionnaire d'évenements */
    Is_Uart_Data();
    Srv_Out_Event();  
    delay(500);
    #ifdef DEBUG
    //Serial.flush(); 
    #endif
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
  digitalWrite(RTSPIN, LOW);         // Libère I2C
  digitalWrite(BUSYPIN, HIGH);        // BUSY = 1 la carte Shield est en pause.
  digitalWrite(LEDVERTE, LOW);        // led status event
  I2C_ClearToSend();                        // libère la ligne I2C
 
  #ifdef DEBUG
  Serial.begin(115200);                // init UART pour debug via USB.
  delay(1000);
  Serial.println(F("START DEBUG ...... "));
  #endif
  
  Wire.begin();                        // init I2C
  Bridge.begin();                      // init Bridge
  FileSystem.begin();                  // init file system 

  I2C_RequestToSend();
  RTC.writeSqwPinMode(OFF);           // led off de la RTC au demarrage
  I2C_ClearToSend();  
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
void I2C_RequestToSend(void)
{
    uint8_t tmp;    
    
    tmp = digitalRead(CTSPIN);
    
    if(tmp)
    {
      while(tmp)
      {
        tmp = digitalRead(CTSPIN);
        delay(100);
      } 
    }
    
    digitalWrite(RTSPIN, HIGH);
    
}
/*******************************************************************************
*	void Drv324p_ClearToSend(void)
Libère la ligne RTS
*******************************************************************************/
void I2C_ClearToSend(void)
{
    delay(100);
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
  shell.begin(F("/root/python/config.py"));
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
    I2C_RequestToSend();    
    DateTime now = RTC.now(); 
    I2C_ClearToSend();    
    
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
dans le tableau date_array.
calcul de l'heure d'été 
in : format
out : pointeur vers date_array (variable globale)
************************************************************/
char *getdate(char format)
{
  
  for(char i=0;i<DATE_STRING_SIZE;i++)
  {
    date_array[i] = 0;
  }
    
  I2C_RequestToSend();
  DateTime now = RTC.now();
  //calcul now DST time
  uint8_t nowmonth = now.month();
  uint8_t nowday = now.day();
  uint8_t nowdow = now.dayOfWeek();
  uint8_t isdst=0;
    
  if (nowmonth < 3 || nowmonth > 10)  isdst= 0U; 
  else if (nowmonth > 3 && nowmonth < 10)  isdst= 1U;
  else if (nowmonth == 3) 
  { 
    int previousSunday = nowday - nowdow;
    isdst= (previousSunday >= 25);
  }
  else if (nowmonth == 10)
  {
   int previousSunday = nowday - nowdow;
   isdst = (previousSunday < 25);
  }
  
  if(format==DATE_ISO8601)
  {
    String str = String(now.year(), DEC); 
    str += '-';
    str += String(now.month(), DEC);
    str += '-';
    str += String(now.day(), DEC);  
    str += 'T';  
    if(isdst==0) str += String(now.hour()-1U, DEC); 
    else if(isdst==1) str += String(now.hour()-2U, DEC);
    str += ':';
    str += String(now.minute(), DEC);
    str += ':'; 
    str += String(now.second(), DEC); 
    
    if(isdst==0) str += String("+0100");
    else if(isdst==1) str += String("+0200");
    
    str.toCharArray(date_array,25);
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
  }
  
  else if(format==UNIX_TIME)
  {
    String str = String(now.unixtime(), DEC); 
    str.toCharArray(date_array,25);
  }  
  
  else if(format==DOW)
  {
    String str = String(now.dayOfWeek(), DEC); 
    str.toCharArray(date_array,25);
  }   
  I2C_ClearToSend();
  return date_array; 
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
    

    jsonvalue = run_python_script_config("sampling_interval"); // lecture du fichier config.json 
    
    I2C_RequestToSend();
    RTC.writenvram(DS1338_NVRAM_REG_SAMPLING,jsonvalue);             // periode d'échantillonnage de la teleinfo pour le shield (en minutes) 
    delay(100);                                                      // ettend un peu entre chauqye w/r
    jsonvalue =  RTC.readnvram(DS1338_NVRAM_REG_SAMPLING);           // affiche le contenu de la nvram à l'adresse 0 
    I2C_ClearToSend(); 
    
    #ifdef DEBUG 
    Serial.print(F("Nvram Sampling Teleinfo every "));  
    Serial.print(jsonvalue);
    Serial.println(F(" minutes"));    
    #endif
    
    adjust_rtc =run_python_script_config("adjust_rtc");        // recupère l'heure à laquelle on ajuste la RTC avec internet
    
    #ifdef DEBUG 
    Serial.print(F("Adjust RTC every day at "));
    Serial.print(adjust_rtc);
    Serial.println(F(" hour"));    
    #endif
    
    #ifdef DEBUG 
    Serial.println(F("Liberation pin BUSY"));  
    #endif
    
    I2C_RequestToSend();
    RTC.writeSqwPinMode(SquareWave1HZ);	                       // clignotement de la led toutes les secondes
    I2C_ClearToSend();
    
    digitalWrite(BUSYPIN, LOW);        // BUSY = 0 la carte Shield peut demarrer.
    


}
/***********************************************************
uint8_t Is_Uart_Data(void)
Cette fonction va lire la émoire nvram de la rtc
pour vérifier si il y a un message à réceptionner.
************************************************************/
void Is_Uart_Data(void)
{
  uint8_t tmp_nvram=0;
  
    I2C_RequestToSend();
    tmp_nvram = RTC.readnvram(DS1338_NVRAM_REG_UART_RTS_TELEINFO);
    I2C_ClearToSend();
    
    if(tmp_nvram)
     {
      Event.Uart_data_ready=1;
      //Serial.println(F("Data UART available..."));  
     }
}


/////////////////////////////////////////////////////////////////////
// Synchronize clock using NTP
void setClock() {  
  Process p;
  
  Serial.println("Setting clock.");
  
  // Sync clock with NTP
  p.runShellCommand("ntpd -nqp 0.openwrt.pool.ntp.org");
  
  // Block until clock sync is completed
  while(p.running());
}
/////////////////////////////////////////////////////////////////////
// Return a string date format
// example:  date -d @1430665070 +"%d-%m-%Y %T %z"
// return : 03-05-2015 16:57:50 +0200
// in : uint long : epoch unix time
// out : char * (pointer to string)

char  *epochinTime(unsigned long millisAtEpoch) 
{
  char epochCharArray[50] = "";
  char buf[12];
  ltoa(millisAtEpoch,buf,10);

  String stra = "@";
  String strb(buf);
  stra +=strb;

  String fmt = "+\"%d-%m-%Y %T %z\"";
  
  Process time;   
  time.begin("date");
  time.addParameter("-d");
  time.addParameter(stra);
  time.addParameter(fmt);
  time.run();
   
  while (time.available() > 0) 
  {
    time.readString().toCharArray(epochCharArray, 50);
  }
  
  return epochCharArray;
  
}
/////////////////////////////////////////////////////////////////////
// Return timestamp of Linino
// in : none
// out :unsigned long

unsigned long timeInEpoch()
{
  Process time;                   // process to run on Linuino
  char epochCharArray[12] = "";   // char array to be used for atol

  // Get UNIX timestamp
  time.begin("date");
  time.addParameter("+%s");
  time.run();
  
  // When execution is completed, store in charArray
  while (time.available() > 0) {
    time.readString().toCharArray(epochCharArray, 12);
  }
  
  // Return long with timestamp
  return atol(epochCharArray);
}
/////////////////////////////////////////////////////////////////////
// calc if we are in a DST time
// in : none
// out :uint8_t
uint8_t IsDst(void)
{
  I2C_RequestToSend();
  DateTime now = RTC.now();

  uint8_t nowmonth = now.month();
  uint8_t nowday = now.day();
  uint8_t nowdow = now.dayOfWeek();
    
    if (nowmonth < 3 || nowmonth > 10)  return 0U; 
    if (nowmonth > 3 && nowmonth < 10)  return 1U; 

    int previousSunday = nowday - nowdow;

    if (nowmonth == 3) return previousSunday >= 25;
    if (nowmonth == 10) return previousSunday < 25;
    
   I2C_ClearToSend();
   
   return 0U; // this line never gonna happend
}
