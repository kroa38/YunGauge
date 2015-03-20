
#include "typedefs.h"
#include "ScriptMain.h"
#include "DrvTwi.h"
#include "Drv324p.h"
#include "Srv_Out.h"


/*******************************************************************************************************
*	void Script_2D(Int8U Dev_address ,const Int8U *tab, Int8U size )
********************************************************************************************************/
void Script_2D(Int8U Dev_address ,Int8U const __flash *tab, Int8U size )
{
static Int8U index;

  for (  index=0U; index<size; index=index+2U)
  {
         DrvTwi_Write_Byte(Dev_address,tab[index],tab[index+1U]);
         __delay_cycles(_1_MILLISECONDE);
         __watchdog_reset();
  }

}
/*******************************************************************************************************
*	void Script_3D(const Int8U *tab, Int8U size )
********************************************************************************************************/
void Script_3D(Int8U const __flash *tab, Int8U size )
{
  static Int8U index;
  
  for ( index=0U; index<size; index=index+3U)
  {
         DrvTwi_Write_Byte(tab[index],tab[index+1U],tab[index+2U]);
         __delay_cycles(_1_MILLISECONDE);
         __watchdog_reset();        
  }

}
