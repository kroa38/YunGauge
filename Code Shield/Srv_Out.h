
#ifndef SRVOUT_H
#define SRVOUT_H

#define LENGHT_HCHC 14
#define LENGHT_HCHP 14
#define LENGHT_PTEC 9

#define COUNTER_TIC_SECONDE   55    // 55 initialement
#define TRAMEFILTERCOUNT  15
typedef volatile struct Struct_Srv_Event{

  volatile Int8U bTrameFilteredOk:1U; 
  volatile Int8U bTrameFilteredNotOk:1U; 
  volatile Int8U bDoorEvent:1U;
    volatile Int8U boidevent:5U;  
}Struct_Srv_Event;



void Srv_Out_Event(void);
void Srv_Out_Event_RTC_TIC(void);
void Srv_Out_Event_Sensor(void);
void Srv_Out_Event_Filter_Teleinfo(void);
void Srv_Out_Event_Send_Teleinfo_to_Arduino(void);
void Srv_Out_Event_Send_Door_to_Arduino(void);
void Srv_Out_Event_Restart_Teleinfo_Capture(void);
void teststring(void);

void Calc_Diff(Int32U *previous_index, char *ptr_index, char *string_out);
void testcalc(void);
void testcalc_shift(void);
void testcat(void);
#endif
