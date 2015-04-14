/***********************************************************
void Srv_Out_Event(void)
Fonction de gestion des evenements
************************************************************/
void Srv_Out_Event(void)
{
   Srv_WaitForLinino();
   Srv_PingGoogle();
   Srv_Ntp_To_Rtc_Update();
   Srv_PrintRtcDate();
   Srv_StoreEventTeleinfoToFile();
   Srv_StoreEventDoorToFile();
   Srv_AdjustDateEveryDay();
   
}
/***********************************************************
void WaitForLinino()
attente de demarrage de Linino
************************************************************/
void Srv_WaitForLinino()
{
  uint8_t jsonvalue;
  
  if(Event.WaitForLinino)
  {
    Event.WaitForLinino=0;
    
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
    Event.PingGoogle = 1;  // on peut pinger google pour mettre à jour l'heure
    ClearToSend();  
  }
}

/**********************************************************************
void Srv_PingGoogle(void) 
permet de savoir si internet est On ou OFF
le script en python envoie par email l'adresse IP
si le resultat est OK alors on met a jour la RTC avec l'heure internet
In: void
Out : uint8_t (1 : internet ON   0: Internet OFF)
************************************************************************/
uint8_t Srv_PingGoogle(void) 
{
  if(Event.PingGoogle)
  {
    uint8_t check_internet;
    Event.PingGoogle=0;
    check_internet = run_python_script_config("check_internet");
      
    #ifdef DEBUG
    Serial.print(F("Google Ping Result : ")); 
    Serial.println(check_internet);
    #endif
    
    if(check_internet == 0)
    {      
      #ifdef DEBUG
      Serial.println(F("No internet connexion !"));
      #endif
      Event.Ntp_To_Rtc_Update = 0;
    }
    else if(check_internet == 1)
    {
      #ifdef DEBUG
      Serial.println(F("Internet is UP"));     
      #endif
      Event.Ntp_To_Rtc_Update = 1;
    }
    return check_internet;
  }

}


/***********************************************************
void Ntp_To_Rtc_Update();
Récupère la date et l'heure de Linino
et met à jour la RTC
************************************************************/
void Srv_Ntp_To_Rtc_Update()
{
     
  if(Event.Ntp_To_Rtc_Update)
  {

    Event.Ntp_To_Rtc_Update=0;
    
    int dt[6];
    String DateL = "";
    String TimeL = "";
    #ifdef DEBUG
    Serial.println(F("Ntp to RTC Update"));
    #endif
    //RTC.begin();
    //interruption de 1Hz pour le shield
    //RTC.writeSqwPinMode(SquareWave1HZ);	
    
    DateL = ProcExec(F("date"),F("+%Y-%m-%d"));
    #ifdef DEBUG
    Serial.print(F("Linino Date:"));
    Serial.print(DateL);
    #endif
  
    TimeL = ProcExec(F("date"),F("+%T"));
    
    #ifdef DEBUG  
    Serial.print(F("Linino Time:"));
    Serial.print(TimeL);
    #endif
  
    dt[0]=DateL.substring(0,4).toInt();  // year
    dt[1]=DateL.substring(5,7).toInt();  // month
    dt[2]=DateL.substring(8).toInt();    // day
    dt[3]=TimeL.substring(0,2).toInt();  // hour
    dt[4]=TimeL.substring(3,5).toInt();  // min
    dt[5]=TimeL.substring(6).toInt();    // sec
    
    //met la rtc a jour avec la date et l'heure internet.
    RequestToSend();    
    RTC.adjust(DateTime(dt[0],dt[1],dt[2],dt[3],dt[4],dt[5]));
    ClearToSend();

    Blink_Led(10); 
    
    #ifdef DEBUG
    Event.PrintRtcDate=1;
    #endif

    }
}

/***********************************************************
void Srv_PrintLininoDate()
Print sur la console uart la date et l'heure de Linino.
************************************************************/
void Srv_PrintLininoDate()
{
 if(Event.PrintLininoDate)
  {
    Event.PrintLininoDate=0;
 
    String DateL = "";
    String TimeL = "";
    
    DateL = ProcExec(F("date"),F("+%Y-%m-%d %T"));
    TimeL = ProcExec("date","+%T");    

    Serial.print(F("Linino Date: "));
    Serial.println(DateL);
    Serial.print("  ");    
    Serial.println(TimeL);
  }

}
/***********************************************************
void Srv_PrintRtcDate()
Print sur la console uart la date.
************************************************************/
void Srv_PrintRtcDate()
{

 if(Event.PrintRtcDate)
 {   
    Event.PrintRtcDate=0;
    
    RequestToSend();    
    DateTime now = RTC.now(); 
    ClearToSend();    
    
    Event.PrintRtcDate=0;
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
    

    
 }
 
}
/***********************************************************
void Srv_AdjustDateEveryDay()

ajuste la RTC avec l'heure de linino.
************************************************************/
void Srv_AdjustDateEveryDay()
{

  unsigned long currentMillis ;
  
  currentMillis = millis();
 
  if((currentMillis - previousMillis) > HOUR_ADJUST_CHECK) 
  {
     #ifdef DEBUG
      Serial.println(F("HOUR_ADJUST_CHECK..."));
     #endif
    RequestToSend();
    DateTime now = RTC.now();
    ClearToSend();      
    
    // save the last time you update
    previousMillis = currentMillis; 
    
    if(now.hour() == HOUR_ADJUST)
    {
      // on peut pinger google pour mettre à jour l'heure  
      Event.PingGoogle = 1;  
      
      #ifdef DEBUG
      Serial.println(F("Adjust Date and Time from Internet at 20h"));
      #endif
    }
  }
  
}

/***********************************************************
void Srv_StoreEventTeleinfoToFile()

Met en forme la donnée reçu par l'uart avec la date
et copie sur le fichier teleinfo.txt

************************************************************/

void Srv_StoreEventTeleinfoToFile()
{
  
  if(Event.StoreEventTeleinfoToFile)
   {
    Event.StoreEventTeleinfoToFile = 0;
    
    #ifdef DEBUG
    Serial.println(F("Store to file teleinfo.log ..."));
    #endif
    
    File dataFile = FileSystem.open("/mnt/sda1/ftp/teleinfo.log", FILE_APPEND);

    // if the file is available, write to it:
    if (dataFile) 
    {

      #ifdef DEBUG 
      Serial.println(F("File ok...!"));
      #endif
      //construct string
      dataString += ',';
      dataString += getdate(DATE_ISO8601);
      dataFile.print(dataString);
      // store event to file
      dataFile.close();
      
      Blink_Led(2);      
       
    }
    else
    {
      #ifdef DEBUG  
      Serial.print(F("File Error....!"));
      #endif
      while(1)
      {
      Blink_Led(4); 
      delay(50);
      }
    }
    

  }
  
}
/***********************************************************
void Srv_StoreEventDoorToFile()

Met en forme la donnée reçu par l'uart avec la date
et copie sur le fichier door.txt
************************************************************/

void Srv_StoreEventDoorToFile()
{
  if(Event.StoreEventDoorToFile)
   {
    Event.StoreEventDoorToFile = 0;
    
    #ifdef DEBUG
    Serial.println(F("Store to file door.log ..."));
    #endif
    
    File dataFile = FileSystem.open("/mnt/sda1/ftp/door.log", FILE_APPEND);

    // if the file is available, write to it:
    if (dataFile) 
    {

      #ifdef DEBUG 
      Serial.println(F("File ok...!"));
      #endif
      //construct string
      dataString += ',';
      dataString += getdate(DATE_ISO8601);
      dataFile.print(dataString);
      // store event to file
      dataFile.close();
      
      Blink_Led(2);                        
      
    }
    else
    {
      #ifdef DEBUG  
      Serial.print(F("File Error....!"));
      #endif
      while(1)
      {
      Blink_Led(20); 
      delay(40);      
      }
    }
 
    
  }
  
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
  

