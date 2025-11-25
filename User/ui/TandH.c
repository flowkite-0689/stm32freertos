
#include "TandH.h"
#include "ui/alarm_all.h"

void TandH()
{
  debug_init();
  OLED_Init();
  DHT11_Init();
  KEY_Init();
  SysTick_Init(); // 初始化滴时器
  DHT11_Data_TypeDef dhtdata;

  u8 key;

  while (1)
  {
    // 全局闹钟处理 - 在温湿度界面也能处理闹钟
    if (Alarm_GlobalHandler())
    {
      delay_ms(10);
      continue; // 如果正在处理闹钟提醒，跳过温湿度循环的其他部分
    }
    IWDG_ReloadCounter();
    delay_ms(10);
    if ((key = KEY_Get()) != 0)
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
      OLED_Printf_Line(0, "Temperature:%d.%dC ",
                       dhtdata.temp_int, dhtdata.temp_deci);
      OLED_Printf_Line(2, "Humidity:  %d.%d%%",
                       dhtdata.humi_int, dhtdata.humi_deci);
                       // 横向温度计（支持小数：25.5°C → 255）
    int16_t temp_tenth = dhtdata.temp_int * 10 + dhtdata.temp_deci;
    OLED_DrawTempBar_Line1(temp_tenth);

    // 横向湿度条
    OLED_DrawHumidityBar_Line3(dhtdata.humi_int);
    }
    else
    {
      // OLED_Clear_Line(2);
      // OLED_Printf_Line(2, "DHT11 Error!    ");
      // OLED_Printf_Line(3, "Code: %d        ", result);
    }
    OLED_Refresh_Dirty();
    delay_ms(600);
  }
}
