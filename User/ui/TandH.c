
#include "TandH.h"


void TandH()
{
debug_init();
  OLED_Init();
	DHT11_Init();
  KEY_Init();	
  SysTick_Init();	 // 初始化滴时器
  DHT11_Data_TypeDef dhtdata; 

  u8 key;
 
  while (1)
  {
delay_ms(10);
    if (key =KEY_Get())
    {
      if (key == KEY2_PRES)
      {
        printf("exit TandH\r\n");
        return;
      }
      
    }
    
    int result = 0;
    result = Read_DHT11(&dhtdata);

				if (result == 0)
        {
          OLED_Clear_Line(3);
          OLED_Printf_Line(2, "T:%d.%dC H:%d.%d%%",
													 dhtdata.temp_int, dhtdata.temp_deci,
													 dhtdata.humi_int, dhtdata.humi_deci);
                           	
        }else
        {
          OLED_Clear_Line(2);
          OLED_Printf_Line(2, "DHT11 Error!");
					OLED_Printf_Line(3, "Code: %d", result);

        }
	OLED_Refresh_Dirty();
    delay_ms(3000);
  }
  
}