#include <Bridge.h>
#include <Console.h>
#include <FileIO.h>
#include <HttpClient.h>
#include <Mailbox.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>
#include <OneWire.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include "teleinfo.h"


/********************************************************************************
VARIABLES GLOBALES
********************************************************************************/
char date_array[DATE_STRING_SIZE]="";
String dataString= "";
uint8_t adjust_rtc;

char current_day,current_hour;
unsigned long previousMillis ;        // will store last time 
unsigned long epochunix = EPOCH_TIME;
struct Evenements
{
     unsigned char Ntp_To_Rtc_Update            : 1;
     unsigned char PingGoogle   		: 1;
     unsigned char UpdateTeleinfoDb             : 1;
     unsigned char UpdateDoorDb                 : 1;  
     unsigned char Ntp_To_RTC_OK        	: 1; 
     unsigned char Uart_data_ready              : 1;
     unsigned char Test_Mode	                : 1;  
     unsigned char RTC_To_Linino_Update         : 1;     
};

Evenements Event = {0};
RTC_DS1338 RTC;
SoftwareSerial mySerial(10, 11); // RX, TX
OneWire ds(8);  //DS18B20 Temperature chip i/o 8

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
    Is_Uart_Data();
    Srv_Out_Event();  
    delay(500);
}
/********************************************************************************
FONCTION GeneralInit()
********************************************************************************/
void GeneralInit(void)
{
  
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
  Event.RTC_To_Linino_Update = 0;     // mise a jour de l'heure linino avec l'heure rtc
  Event.UpdateTeleinfoDb = 0;
  Event.UpdateDoorDb = 0;  
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
void PrintRtcDate(void)
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
void *get_rtc_date()

renvoie la date au format ISO8601 ou CUSTOM
dans le tableau date_array.
calcul de l'heure d'été possible
in : format
out : pointeur vers date_array (variable globale)
************************************************************/
char *get_rtc_date(char format)
{
  
  for(char i=0;i<DATE_STRING_SIZE;i++)
  {
    date_array[i] = 0;
  }
    
  I2C_RequestToSend();
  DateTime now = RTC.now();

  
  if(format==DATE_ISO8601)  //ex : 2012-07-09T22:27:50
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
  
  else if(format==UNIX_TIME)  // insert unix epoch time in second
  {
    String str = String(now.unixtime(), DEC); 
    str.toCharArray(date_array,25);
  }  
  
  else if(format==DOW)	// insert day of week
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
void PrintLininoDate(void)
{
    #ifdef DEBUG
    String DateL = "";
    String TimeL = "";
    
    DateL = ProcExec(F("date"),F("+%Y-%m-%d %T"));
    TimeL = ProcExec("date","+%T");    

    Serial.print(F("Linino Date: "));
    Serial.println(DateL);
    //Serial.print("  ");    
    //Serial.println(TimeL);
    #endif
}
/***********************************************************
void WaitForLinino()
attente de demarrage de Linino ~50s
************************************************************/
void WaitForLinino(void)
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
    
    if( jsonvalue >= 200)
    {
      #ifdef DEBUG
      Serial.print(F("TEST mode : Sampling every  ")); 
      uint8_t tmp = jsonvalue-200 ;
      Serial.print(tmp,DEC);
      Serial.println(F(" secondes"));
      #endif
      Event.Test_Mode=1;
    } 
   else
    {
      #ifdef DEBUG
      Serial.print(F("Normal mode : Sampling every  "));  
      Serial.print(jsonvalue);
      Serial.println(F(" minutes"));
      #endif
      Event.Test_Mode=0;
    }  
  
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
    RTC.writeSqwPinMode(SquareWave1HZ);	 // clignotement de la led toutes les secondes
    I2C_ClearToSend();
    
    rtc_to_linino_date_update();        // update Linino Date from RTC
    
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
     }
}


/***********************************************************
void SyncLininoClock(void)
// Synchronize linino clock using NTP
************************************************************/
void SyncLininoClock(void)
{  
  Process p;
  
  Serial.println("Setting clock.");
  
  // Sync clock with NTP
  p.runShellCommand("ntpd -nqp 0.openwrt.pool.ntp.org");
  
  // Block until clock sync is completed
  while(p.running());
}

/***********************************************************
char  *epoch_to_iso8601(unsigned long millisAtEpoch)

Return a string date format
example:  date -d @1430665070 +"%d-%m-%YT%T"
return : 03-05-2015T16:57:50
in : uint long : epoch unix time
out : char * (pointer to string)
************************************************************/
char  *epoch_to_iso8601(unsigned long millisAtEpoch) 
{
  char epochCharArray[25] = "";
  char buf[12];
  char i=0;
  ltoa(millisAtEpoch,buf,10);

  String stra = "@";
  String strb(buf);
  stra +=strb;

  String fmt = "+\"%Y-%m-%dT%T\"";
  Process time;   
  time.begin("date");
  time.addParameter("-d");
  time.addParameter(stra);
  time.addParameter(fmt);
  time.run();
   
  while (time.available() > 0) 
  {
    char c = time.read();
    if ( (c != '"') && (c != 0x0A) && (c != 0x0D ))
      epochCharArray[i++] = c;
  }
  
  return epochCharArray;
  
}
/***********************************************************
unsigned long timeInEpoch(void)
Return timestamp of Linino
in : none
out :unsigned long  (ex 1432023044)
************************************************************/
unsigned long timeInEpoch(void)
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
/***********************************************************
void rtc_to_linino_date_update(void)
Update the Linino date with the RTC date only if rtc date
is superior to linino date. This process is executed 1 time
at  the end of waitforlinino() function
in : none
out : none
************************************************************/
void rtc_to_linino_date_update(void)
{  
    Process time;                   // process to run on Linuino
    
    I2C_RequestToSend();
    DateTime now = RTC.now();
    I2C_ClearToSend(); 
    uint8_t isdst=Is_DST_Time();
    
    unsigned long rtc_epoch = now.unixtime();  // retrieve epoch time from RTC
    unsigned long linino_epoch = 0UL;
    unsigned long linino_tz = 0UL;
    unsigned long diff_epoch;
    linino_epoch = ProcExec(F("date"),F("+%s")).toInt();
    linino_tz = ProcExec(F("date"),F("+%z")).substring(1,3).toInt();  // retrieve time zone
    linino_epoch += linino_tz*3600;
    
    #ifdef DEBUG
    Serial.print("Linino EPOCH : ");
    Serial.println(linino_epoch);
    Serial.print("RTC EPOCH : ");
    Serial.println(rtc_epoch,DEC);
    #endif
    
    if (rtc_epoch > linino_epoch)
      diff_epoch = rtc_epoch - linino_epoch;
    else
      diff_epoch = linino_epoch - rtc_epoch;
    
    if (diff_epoch > 60)  // si ecart de plus de 1 minute alors mise à jour.
    {          
      if(isdst) 
      {
        rtc_epoch -= 7200;    // substract 2 hours
      }
      else
      {
        rtc_epoch -= 3600;    // substract 1 hour
      }  
      
      String str = "@" + String(rtc_epoch, DEC);
      // Set UNIX timestamp

      time.begin("date");
      time.addParameter("-s");
      time.addParameter(str);
      
      time.run();  
      #ifdef DEBUG
      Serial.println(F("Linino Date Updated with RTC"));
      #endif
    }  
}
/***********************************************************
uint8_t Is_DST_Time(void)
Calculate DST ( 1 = YES ,  0 = NO ) from the RTC 
in : none
out : uint8_t
************************************************************/
uint8_t Is_DST_Time(void)
{
  I2C_RequestToSend();
  DateTime now = RTC.now();
  I2C_ClearToSend();
  
  //calcul now DST time
  uint8_t nowmonth = now.month();
  uint8_t nowday = now.day();
  uint8_t nowdow = now.dayOfWeek();
  uint8_t isdst=0;
  
  //////////////////////////////////////////////  
  // calcul si on est en heure d'été ou d'hiver  
  //////////////////////////////////////////////

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

  return isdst;
}
/***********************************************************
String temperature(void)
Return temperature from attached DS18B20 sensor 
in : none
out : String (ex -12.2 or 27.1 or 99 for error)
************************************************************/
String temperature(void)
{

  byte data[2];
  byte addr[8];
  word Tc_100;
  word Whole;
  word Fract;

  ds.reset_search();
  
  if ( !ds.search(addr)) {
      Serial.print("No more addresses.\n");
      ds.reset_search();
      return "99.9";
  }
  
   if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return "99.9";
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( byte i = 0; i < 2; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
 
  word LowByte = data[0];
  word HighByte = data[1];
  word TReading = (HighByte << 8) + LowByte;
  word SignBit = TReading & 0x8000;  // test most sig bit
  
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;

  String Result = "";
  
   if (SignBit) // If its negative
  {
     Result = "-";
  }
  Result += String(Whole,DEC);
  Result += ".";
  
  if (Fract < 10)
  {
     Result += "0";
  }
  Result += String(Fract,DEC).substring(0,1);

  return Result; 
}
/***********************************************************
void Reset_Shield(void)
Reset le shield
Utilisé tous les jour au moment de la mise à l'heure de la RTC
suite a un bug pas encore resolu du shield qui ne captrue plus
la teleinfo après 11j de capture.
in : none
out : none
************************************************************/
void Reset_Shield(void)
{
  
    digitalWrite(BUSYPIN, HIGH);   // BUSY = 1 la carte Shield est initialisé.
    delay(1000);
    digitalWrite(BUSYPIN, LOW);    // BUSY = 0 la carte Shield peut demarrer.  
  
}
/***********************************************************
void Reset_by_Watchdog(void)
in : none
out : none
************************************************************/
void Reset_by_Watchdog(void)
{
  wdt_enable(WDTO_1S);
  while(1);
  
}
