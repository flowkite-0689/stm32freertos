#include "flashlight.h"
#include "ui/alarm_all.h"


void flashlight()
{
  LED_Init();
  KEY_Init();
  OLED_Init();
  LED_Set_All(1);
  OLED_ShowPicture(0,16,32,32,gImage_flashlight,1);
 OLED_Printf_Line(0,"KEY0=ON");
 OLED_Printf_Line(3,"light OFF");
  OLED_Refresh();
  OLED_Refresh_Dirty();
  u8 key ;
  while (1)
  {
    // 全局闹钟处理 - 在手电筒界面也能处理闹钟
    if (Alarm_GlobalHandler()) {
        delay_ms(10);
        continue; // 如果正在处理闹钟提醒，跳过循环的其他部分
    }
    
    delay_ms(10);
    if ((key =KEY_Get())!=0)
    {
      switch (key)
      {
      case KEY0_PRES:
        LED0 = 0;
        OLED_Printf_Line(0,"KEY1=off ");
        OLED_ShowPicture(65,16,32,32,gImage_sun,1);
        OLED_Printf_Line(3,"light ON  ");
        OLED_Refresh_Dirty();
        break;
        case KEY1_PRES:
        LED0 = 1;
        OLED_Printf_Line(0,"KEY0= ON");
        OLED_ShowPicture(65,16,32,32,gImage_moon,1);
        OLED_Printf_Line(3,"light OFF");
        OLED_Refresh_Dirty();
        break;
        case KEY2_PRES:
        LED0=1;
        return;
       
      
      default:
        break;
      }
    }
    
  }
  
  
}
