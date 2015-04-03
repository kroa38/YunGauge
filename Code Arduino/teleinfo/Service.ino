/***********************************************************
void Srv_Out_Event(void)
Fonction de gestion des evenements
************************************************************/
void Srv_Out_Event(void)
{
   Srv_WaitForLinino();
   Srv_PingGoogle();
   Srv_Ntp_To_Rtc_Update();
   Srv_Email_Ip_Address();
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
    Serial.println("");
    Serial.println(F("Linux is ready "));
    #endif
    RequestToSend();
    RTC.writeSqwPinMode(SquareWave1HZ);	
    ClearToSend();
    
    digitalWrite(BUSYPIN, LOW);        // BUSY = 0 la carte Shield peut demarrer.
    Event.PingGoogle = 1;  // on peut pinger google pour mettre à jour l'heure
  }
}

/***********************************************************
String pingGoogle(void) 
Ping pour connaitre l'etat d'internet
retourne 1 si internet ok sinon retourne 0
************************************************************/
char Srv_PingGoogle(void) 
{
  if(Event.PingGoogle)
  {
    Event.PingGoogle=0;
    
    Process shell;
    String result="";
    char retval;
    
    #ifdef DEBUG
    Serial.println(F("Run shell script ping")); 
    #endif    

    shell.begin(F("/mnt/sda1/shell/pingoo.sh"));
    shell.run();

    while(shell.available())
    {
      result += (char)shell.read();
    }
    Serial.flush();
   
    #ifdef DEBUG
    Serial.print(F("Google Ping Result : ")); 
    Serial.println(result[0]);
    #endif
    
    if(result[0] == '0')
    {
      
      #ifdef DEBUG
      Serial.println(F("No internet connexion !"));
      Event.PrintRtcDate=1;
      #endif
    
      retval=0;
    }
    else if(result[0] == '1')
    {
      #ifdef DEBUG
      Serial.println(F("Internet Ok"));     
      #endif
      
      retval=1;
      Event.Ntp_To_Rtc_Update = 1;
    }
    
    return(retval);
  }
  else
  {
    return(0);
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
    Event.Email_Ip_Address=1;
    Blink_Led(10); 
    
    #ifdef DEBUG
    Event.PrintRtcDate=1;
    #endif

    }
}
/***********************************************************
char Srv_Email_Ip_Address(void)
envoie l'adress ip du yun par email
************************************************************/
void Srv_Email_Ip_Address(void) 
{
  if(Event.Email_Ip_Address)
  {
    Event.Email_Ip_Address=0;
    
    Process shell;
    String result="";
    char retval;
    
    #ifdef DEBUG
    Serial.println(F("Email Yun IP address")); 
    #endif    

    shell.begin(F("/mnt/sda1/shell/my_ip.sh"));
    shell.run();
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
    
    File dataFile = FileSystem.open("/mnt/sda1/log/teleinfo.log", FILE_APPEND);

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
    
    File dataFile = FileSystem.open("/mnt/sda1/log/door.log", FILE_APPEND);

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
  

