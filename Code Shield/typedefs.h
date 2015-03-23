
#ifndef _typedefs_H_
#define _typedefs_H_

#define Int32U unsigned long
#define Int16U unsigned int
#define Int8U  unsigned char

#define SET        1U
#define CLEAR      0U
#define TRUE       1U
#define FALSE      0U
#define EEPROM_SIZE 21U           

/******************************************************/
/* kroa38 */


/* description A block of code may be made atomic by wrapping it with this
 macro.  Something which is atomic cannot be interrupted by interrupts */

#define ATOMIC(code)                      \
  {                                       \
    Int8U em_isr_state;                   \
    em_isr_state = __save_interrupt();    \
    __disable_interrupt();                \
    do{ code }while(0U);                  \
    __restore_interrupt( em_isr_state );  \
  }
#endif

