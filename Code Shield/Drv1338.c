
#include "typedefs.h"
#include "DrvTwi.h"
#include "Drv1338.h"
#include "ScriptMain.h"
#include "Scripts_1338.h"

Int8U previous_sampling_time=99U;

/*******************************************************************************
*	Int8U Drv1338_Init(void)

Cette fonction initialise le switch HDMI AD1338

*******************************************************************************/
void Drv_DS1338_Init(void)
{
     
  Script_3D(SCRIPT_INIT_DS1338,SIZEOF_SCRIPT_INIT_DS1338);
  
}
/*******************************************************************************
*	Int8U Drv_DS1338_read_minute(void)
Retourne les minutes de la RTC
*******************************************************************************/
Int8U Drv_DS1338_read_minute(void)
{
   Int8U tmp1=0U,tmp2=0U;
     
     tmp1 = DrvTwi_Read_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_MINUTE_REG);
     tmp2 = ((tmp1 & DS1338_MASK_10_MIN) >> DS1338_SHIFT_10_MIN)*10;
     tmp1 &= DS1338_MASK_MIN;
     
     return(tmp1+tmp2);
     
}
/*******************************************************************************
*	Int8U Drv_DS1338_read_hour(void)
Retourne les heure de la RTC (00--24)
*******************************************************************************/
Int8U Drv_DS1338_read_hour(void)
{
     Int8U tmp1=0U,tmp2=0U;
     
     tmp1 = DrvTwi_Read_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_HOUR_REG); 
     tmp2 = ((tmp1 & DS1338_MASK_10_HOUR) >> DS1338_SHIFT_10_HOUR)*10;
     tmp1 &= DS1338_MASK_HOUR;
     return(tmp1+tmp2);
}
/*******************************************************************************
*	Int8U Drv_DS1338_read_hour(void)
Retourne les heure de la RTC (00--24)
*******************************************************************************/
Int8U Drv_DS1338_read_date(void)
{
     Int8U tmp1=0U,tmp2=0U;
     
     tmp1 = DrvTwi_Read_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_DATE_REG); 
     tmp2 = ((tmp1 & DS1338_MASK_10_DATE) >> DS1338_SHIFT_10_DATE)*10;
     tmp1 &= DS1338_MASK_HOUR;
     return(tmp1+tmp2);
}
/*******************************************************************************
*	Int8U Drv_DS1338_read_hour(void)
Retourne le jour en cours
*******************************************************************************/
Int8U Drv_DS1338_read_day(void)
{
     Int8U tmp1=0U,tmp2=0U;
     
     tmp1 = DrvTwi_Read_Byte(I2C_DEVICE_ADDR_DS1338,DS1338_DAY_REG); 
     tmp2 = tmp1 & DS1338_MASK_DAY ;
     return(tmp2);
}

/*******************************************************************************
Int8U Drv_DS1338_Check_Minute(void)
permet de synchroniser sur l'horloge la prise des echantillons de teleinfo
soit toutes les 1min, 5min, 10min, 15min, 20min, 30min, 60min.tous les jours 
à heure fixe

pour toutes les minutes passer 1 en paramètre.
pour toutes les 20 minutes passer 20 en paramètre
pour tous les jours à 6h passer le paramètre 106
pour tous les jours à 14h passer le paramètre 114

In : mode en minute

out : 1 ok on est bien synchronisé sur le temps
out : 0 on est pas synchronisé.


*******************************************************************************/
Int8U Drv_DS1338_Synchro_With_RTC(Int8U sampling_rate)
{
 
  Int8U tmp=0U;
  Int8U retval=0U;
  
  //Drv324p_RequestToSend();              // attente que la ligne CTS soit libre
  if(sampling_rate == SAMPLING_EVERY_MIN)
  {  
      tmp=Drv_DS1338_read_minute();
      
      if( (tmp%SAMPLING_EVERY_MIN) == 0 )
      {
        if(  previous_sampling_time != tmp)
        {
          retval = 1U;
          previous_sampling_time = tmp;
        }
        else
        {
          retval = 0U;
        }
      }
  }  
  else if(sampling_rate == SAMPLING_EVERY_5MIN)
  {  
      tmp=Drv_DS1338_read_minute();
      
      if( (tmp%SAMPLING_EVERY_5MIN) == 0 )
      {
        if(  previous_sampling_time != tmp)
        {
          retval = 1U;
          previous_sampling_time = tmp;
        }
        else
        {
          retval = 0U;
        }
      }
  }
  else if(sampling_rate == SAMPLING_EVERY_15MIN)
  {  
      tmp=Drv_DS1338_read_minute();
      
      if( (tmp%SAMPLING_EVERY_15MIN) == 0 )
      {
        if(  previous_sampling_time != tmp)
        {
          retval = 1U;
          previous_sampling_time = tmp;
        }
        else
        {
          retval = 0U;
        }
      }
  }
  else if(sampling_rate == SAMPLING_EVERY_20MIN)
  {  
      tmp=Drv_DS1338_read_minute();
      
      if( (tmp%SAMPLING_EVERY_20MIN) == 0 )
      {
        if(  previous_sampling_time != tmp)
        {
          retval = 1U;
          previous_sampling_time = tmp;
        }
        else
        {
          retval = 0U;
        }
      }
  } 
  else if(sampling_rate == SAMPLING_EVERY_30MIN)
  {  
      tmp=Drv_DS1338_read_minute();
      
      if( (tmp%SAMPLING_EVERY_30MIN) == 0 )
      {
        if(  previous_sampling_time != tmp)
        {
          retval = 1U;
          previous_sampling_time = tmp;
        }
        else
        {
          retval = 0U;
        }
      }
  }
  else if(sampling_rate == SAMPLING_EVERY_60MIN)
  {  
      tmp=Drv_DS1338_read_hour();

        if(  previous_sampling_time != tmp)
        {
          retval = 1U;
          previous_sampling_time = tmp;
        }
        else
        {
          retval = 0U;
        }
  }      
        
  else if(sampling_rate >= SAMPLING_EVERY_DAY)
  {  
      tmp=Drv_DS1338_read_hour();
      
      if(tmp==(sampling_rate-100U))                   // on recupère l'heure
      {
        tmp=Drv_DS1338_read_date();
        
        if(  previous_sampling_time != tmp)
        {
          retval = 1U;
          previous_sampling_time = tmp;
        }
        else
        {
          retval = 0U;
        }
      }
  }  
  
  //Drv324p_ClearToSend();
  
  return(retval);
  
}