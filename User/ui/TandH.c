#include "TandH.h"
#include "ui/alarm_all.h"
// 温度进度条（line=1）
void OLED_DrawTempBar_Line1(int16_t temp_tenth) // 0.1°C
{
    OLED_Clear_Line(1);
    // 标签
    OLED_ShowString(0, 16, (uint8_t*)"0C", 12, 1);
    OLED_ShowString(110, 16, (uint8_t*)"50C", 12, 1);
    // 进度条：x=20, y=18, w=88, h=8, 0~500 (0.0~50.0°C)
    OLED_DrawProgressBar(17, 18, 87, 8, temp_tenth, 0, 500, 1, 1);
}

// 湿度进度条（line=3）
void OLED_DrawHumidityBar_Line3(uint8_t humi)
{
    OLED_Clear_Line(3);
    OLED_ShowString(0, 48, (uint8_t*)"0", 12, 1);
    OLED_ShowString(110, 48, (uint8_t*)"100", 12, 1);
    OLED_DrawProgressBar(17, 52, 87, 8, humi, 0, 100, 1, 1);
}
void TandH()
{
  debug_init();
  OLED_Init();
  DHT11_Init();
  KEY_Init();
  SysTick_Init(); // 初始化滴时器
  DHT11_Data_TypeDef dhtdata;
   int16_t last_date_T=250;
 int16_t last_date_H=0;
  u8 key;
  u32 last_re_time= get_systick();
  
  while (1)
  {
    int result = 0;
    // 全局闹钟处理 - 在温湿度界面也能处理闹钟
    if (Alarm_GlobalHandler())
    {
      delay_ms(10);
      continue; // 如果正在处理闹钟提醒，跳过温湿度循环的其他部分
    }
    IWDG_ReloadCounter();
    if (get_systick()-last_re_time>=600)
    {
        
    result = Read_DHT11(&dhtdata);
    }
    

    if ((key = KEY_Get()) != 0)
    {
      if (key == KEY2_PRES)
      {
        OLED_Clear();
        printf("exit TandH\r\n");
        return;
      }
    }





    if (result == 0)
    {
      OLED_Clear_Line(3);
      OLED_Printf_Line(0, "Temperature:%d.%dC ",
                       dhtdata.temp_int, dhtdata.temp_deci);
      OLED_Printf_Line(2, "Humidity:  %d.%d%%",
                       dhtdata.humi_int, dhtdata.humi_deci);
                       // 横向温度计（支持小数：25.5°C → 255）
    

    
    }
    else
    {
      // OLED_Clear_Line(2);
      // OLED_Printf_Line(2, "DHT11 Error!    ");
      // OLED_Printf_Line(3, "Code: %d        ", result);
    }
    int16_t temp_tenth = dhtdata.temp_int * 10 + dhtdata.temp_deci;
    if (temp_tenth >last_date_T)
    {
      
     if (temp_tenth-last_date_T>=10)
     {
        last_date_T+=10;
     }
     
        last_date_T++;
      
    }else if (temp_tenth  < last_date_T)
    {
      last_date_T--;
    }
      OLED_DrawTempBar_Line1(last_date_T);

    if (dhtdata.humi_int>last_date_H )
    {
      last_date_H++;
    }else if (dhtdata.humi_int < last_date_H  )
    {
      last_date_H--;
    }
    

    // 横向湿度条
    OLED_DrawHumidityBar_Line3(last_date_H);
    OLED_Refresh_Dirty();
    delay_ms(10);
  }
}
