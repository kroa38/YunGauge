
#ifndef DRVTWI_H
#define DRVTWI_H

#include "DrvTime.h"
#include "typedefs.h"
#include "Drv324p.h"

typedef struct Struct_Twi_Error_Flag {
  Int8U bi2c:1U;
  Int8U bRead:1U;
  Int8U bWrite:1U;  
  Int8U bDS1338S_ChipSet:1U;
}Struct_Twi_Error_Flag;

/****************************************************************************
  TWI Status/Control register definitions
****************************************************************************/
#define TWI_BUFFER_SIZE 4U   // Set this to the largest message size that will be sent including address byte.

#define TWI_TWBR          0x10     // 0x20=100kHz bit rate, 0x10=200kHz, 0x0A=300KHz
#define TWI_TWPS          0x00     //

#define TWI_GEN_CALL         0x00  // The General Call address is 0

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20

// Sample TWI transmission states, used in the main application.
#define SEND_DATA             0x01
#define REQUEST_DATA          0x02
#define READ_DATA_FROM_BUFFER 0x03
#define WRITE_DATA            0x04

/****************************************************************************
  Function definitions
****************************************************************************/
void TWI_Master_Initialise( void );
Int8U DrvTwi_Read_Byte(Int8U,Int8U);
void DrvTwi_Write_Byte(Int8U,Int8U,Int8U);
void Twi_Device_Error_ID(Int8U Device_Addr);
/****************************************************************************
  Bit and byte definitions
****************************************************************************/

#define TRUE                    1U
#define FALSE                   0U
#define TWI_NO_ERROR            0U
#define TWI_ERROR_READ          0xFE
#define TWI_ERROR_WRITE         0xFD
#define TWI_ERROR_READWRITE     0xFC

#define TWI_WRITE               0xFE
#define TWI_READ                0x01


#define I2C_ERROR_8192          1U
#define I2C_ERROR_7513          2U
#define I2C_ERROR_7842          4U
/****************************************************************************
  TWI State codes
****************************************************************************/
// General TWI Master staus codes
#define TWI_START                  0x08  // START has been transmitted
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter staus codes
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been tramsmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been tramsmitted and NACK received
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been tramsmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been tramsmitted and NACK received

// TWI Master Receiver staus codes
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been tramsmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been tramsmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK tramsmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK tramsmitted

// TWI Slave Transmitter staus codes
#define TWI_STX_ADR_ACK            0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST 0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK           0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK          0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE 0xC8  // Last data byte in TWDR has been transmitted (TWEA = “0”); ACK has been received

// TWI Slave Receiver staus codes
#define TWI_SRX_ADR_ACK            0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST 0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK            0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST 0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK      0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK       0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK      0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
#define TWI_NO_STATE               0xF8  // No relevant state information available; TWINT = “0”
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition


#endif
