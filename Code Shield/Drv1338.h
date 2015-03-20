

#ifndef DRV1338_H
#define DRV1338_H

#define SAMPLING_EVERY_MIN     1
#define SAMPLING_EVERY_5MIN     5
#define SAMPLING_EVERY_15MIN    15
#define SAMPLING_EVERY_20MIN    20
#define SAMPLING_EVERY_30MIN    30
#define SAMPLING_EVERY_60MIN    60
#define SAMPLING_EVERY_DAY      100


#define I2C_DEVICE_ADDR_DS1338   0xD0         // I2C device address of DS1338 (HW fixed addr)

#define   DS1338_SECOND_REG      0x00
  #define   DS1338_BIT_CH        7U
  #define   DS1338_MASK_10_SEC   0x70
  #define   DS1338_SHIFT_10_SEC  4U          //decalage à droite à faire
  #define   DS1338_MASK_SEC      0x0F

#define   DS1338_MINUTE_REG      0x01
  #define   DS1338_MASK_10_MIN   0x70
  #define   DS1338_SHIFT_10_MIN  4U          //decalage à droite à faire
  #define   DS1338_MASK_MIN      0x0F

#define   DS1338_HOUR_REG      0x02
  #define   DS1338_MASK_10_HOUR   0x30
  #define   DS1338_SHIFT_10_HOUR  4U          //decalage à droite à faire
  #define   DS1338_MASK_HOUR   0x0F

#define   DS1338_DAY_REG      0x03
  #define   DS1338_MASK_DAY   0x07

#define   DS1338_DATE_REG      0x04
  #define   DS1338_MASK_10_DATE   0x30
  #define   DS1338_SHIFT_10_DATE  4U          //decalage à droite à faire
  #define   DS1338_MASK_DATE   0x0F

#define   DS1338_CONTROL_REG     0x07
  #define   DS1338_BIT_OUT_CONTROL_REG        7U
  #define   DS1338_BIT_OSF_CONTROL_REG        5U
  #define   DS1338_BIT_SQWE_CONTROL_REG       4U
  #define   DS1338_BIT_RS1_CONTROL_REG        1U
  #define   DS1338_BIT_RS0_CONTROL_REG        0U

#define   DS1338_NVRAM_REG     0x08

void Drv_DS1338_Init(void);
Int8U Drv_DS1338_read_minute(void);
Int8U Drv_DS1338_read_hour(void);
Int8U Drv_DS1338_read_date(void);
Int8U Drv_DS1338_read_day(void);
Int8U Drv_DS1338_Synchro_With_RTC(Int8U sampling_rate);

#endif
