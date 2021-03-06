/***********************************************************
void Srv_Out_Event(void)
Fonction de gestion des evenements
************************************************************/
void Srv_Out_Event(void)
{
   Srv_read_uart_data();
   Srv_AdjustDateEveryDay();   
   Srv_PingGoogle();
   Srv_Ntp_To_Rtc_Update();
   Srv_UpdateTeleinfoDb();  
}
/***********************************************************
void read_uart_data(void)
ledcture du buffer de l'uart
************************************************************/
void Srv_read_uart_data(void)
{
  if(Event.Uart_data_ready)
  {
      Event.Uart_data_ready=0;
      char nb=0U;
      dataString = "";
      char MsgChar;
      char lastchar;
     
      unsigned long currentMillis;

      mySerial.begin(19200);               // réception de la téléinfo à 19200 bauds  
      
      I2C_RequestToSend();
      RTC.writenvram(DS1338_NVRAM_REG_UART_RTS_TELEINFO,0U);
      I2C_ClearToSend();
      currentMillis = millis(); 
      
      while(millis()-currentMillis < 500)      // boucle 500ms mini
      {
        while (mySerial.available())            // boucle si reception de caractères
        { 
           MsgChar=char(mySerial.read());
           if (MsgChar != 0U) lastchar=MsgChar;   // le dernier caractère contient le crc
            
           if( (MsgChar>31) && (MsgChar<127))    // on ne prend pas en compte tous les caractères
           {
             dataString += MsgChar;
             nb+=1;
           }
        }
      }
      
      mySerial.end();
      mySerial.flush();
      
      if(nb)
      {
          /* calcul du CRC de la trame reçue */
          char strln=0,crc=0;
          strln = dataString.length();
          
           for ( char i=0; i<strln-1; i++ )
          {
            crc = (crc + dataString.charAt(i)) & 0x3F;
          }
          crc = crc + 0x20;

          if(crc == lastchar)
          {
            dataString.setCharAt(strln-1,0x20);  // supress 'crc char'
            dataString += temperature();

            #ifdef DEBUG
            Serial.println(dataString);
            #endif
          }

        
         // Si CRC ok on autorise l'écriture sur fichier
         if(crc == lastchar)
         {
            if(dataString.startsWith(F("DOOR")))
            {
              Event.UpdateDoorDb = 1;
              Blink_Led(2);
            }
            else
            {
              Event.UpdateTeleinfoDb = 1;
              Blink_Led(1);
            }
         }
      }

      
   }
}
/***********************************************************
void Srv_AdjustDateEveryDay()
ajuste la RTC avec l'heure de linino.
si heure deja ajusté on check toutes les 50minutes
sinon on retente toute les 2 minutes
************************************************************/
void Srv_AdjustDateEveryDay(void)
{

  unsigned long currentMillis,adjust_at ;
  
  if(Event.Ntp_To_RTC_OK)              // si heure deja ajusté
    adjust_at = HOUR_ADJUST_CHECK;     // on check toutes les 50minutes
  else
    adjust_at = HOUR_ADJUST_CHECK_THIN;    // sinon on retente toute les 2 minutes
  
  currentMillis = millis();
 
  if((currentMillis - previousMillis) > adjust_at) 
  {        
      if(Event.Ntp_To_RTC_OK) 
       { 
          I2C_RequestToSend();
          DateTime now = RTC.now();
          I2C_ClearToSend();      
          
          // save the last time you update
          previousMillis = currentMillis; 
          
          if(now.hour() == adjust_rtc)
          {
              //Reset_Shield();         
              // on peut pinger google pour mettre à jour l'heure  
              Event.PingGoogle = 1;  
              
              #ifdef DEBUG
              Serial.println(F("Try to update RTC from Internet..."));
              #endif
          }
      }
      else
      {
        previousMillis = currentMillis;
        Event.PingGoogle = 1;  
        #ifdef DEBUG
        Serial.println(F("Try to update RTC from Internet..."));
        #endif
      }
        
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
      Event.Ntp_To_Rtc_Update = 0;   
      #ifdef DEBUG
      Serial.println(F("No internet connexion !"));
      PrintRtcDate();
      PrintLininoDate();
      #endif      
    }
    else if(check_internet == 1)
    {
      Event.Ntp_To_Rtc_Update = 1;
      #ifdef DEBUG
      Serial.println(F("Internet is UP"));     
      #endif
    }
    return check_internet;
  }

}
/**********************************************************************
uint8_t isConnectedToInternet()
permet de savoir si internet est On ou OFF
utilise un process shell (plus rapide qu'en python)
In: void
Out : uint8_t (1 : internet ON   0: Internet OFF)
************************************************************************/
uint8_t isConnectedToInternet(void) 
{
  if(Event.PingGoogle)
  {
    Event.PingGoogle=0;
    
  #ifdef DEBUG
  Serial.print(F("Check Internet"));     
  #endif
  
  Process p;
  uint8_t result=0;
  
  // Check if ping to google.com is successful
  p.runShellCommand("ping -W 1 -c 4 www.google.com >& /dev/null && echo 1 || echo 0");
  // Wait until execution is completed, return result
  while(p.running()); 
  result = p.parseInt();
  
  if(result==1)
    {
    #ifdef DEBUG
    Serial.println(F("Internet is UP"));     
    #endif
    Event.Ntp_To_Rtc_Update = 1;
    }
  else if(result==0)
    {
    #ifdef DEBUG
    Serial.println(F("Internet is DOWN"));     
    #endif      
    Event.Ntp_To_Rtc_Update = 0;  
    }  
   
  return result;
  }
  
}

/***********************************************************
void Ntp_To_Rtc_Update();
Récupère la date et l'heure de Linino
et met à jour la RTC
************************************************************/
void Srv_Ntp_To_Rtc_Update(void)
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
    I2C_RequestToSend();    
    RTC.adjust(DateTime(dt[0],dt[1],dt[2],dt[3],dt[4],dt[5]));
    I2C_ClearToSend();

    Blink_Led(10); 
    Event.Ntp_To_RTC_OK = 1;
    #ifdef DEBUG
    PrintRtcDate();
    #endif

    }
}

/***********************************************************
void Srv_UpdateTeleinfoDb();
Met a jour la base de donnée teleinfo
reset arduino + shield à l'heure forunie par adjust_rtc
afin d'éviter un pb de buffer.
In : String
Out : void
************************************************************/
void Srv_UpdateTeleinfoDb(void)
{ 
    if(Event.UpdateTeleinfoDb)
  {
    Event.UpdateTeleinfoDb = 0;
    String Pytcde = "python /root/python/main.py ";
    Pytcde += dataString;
   
    #ifdef DEBUG 
    Serial.println("Update Database Teleinfo");
    #endif

    Process shell;
    shell.runShellCommand(Pytcde);
    while(shell.running()); 
    
    I2C_RequestToSend();
    DateTime now = RTC.now();
    I2C_ClearToSend();   
    
    if(now.hour() == adjust_rtc)
     {
       //shell.runShellCommand("reboot");
       Reset_by_Watchdog();
     }

  }
}
