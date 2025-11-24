#include "iwdg_test.h"

void iwdg_test(void)
{
  LED_Init();
  KEY_Init();
  SysTick_Init();
  IWDG_Init();

  printf("iwdg_test\r\n");

  OLED_Printf_Line(1, "iwdg_test");

  LED_Set_All(1);
  u8 key;
  
  uint32_t d1_last_time = get_systick();
  uint32_t d2_last_time = get_systick();
  uint32_t feed_last_time = get_systick();
  
  uint32_t current_time;
int count =0;
  while (1)
  {
    current_time = get_systick();
    
    // D1 0.5秒改变一次状态
    if (current_time - d1_last_time >= 500) {
      LED_Toggle(1);
      d1_last_time = current_time;
    }
    
    // D2 0.8秒改变一次状态
    if (current_time - d2_last_time >= 800) {
      LED_Toggle(2);
      d2_last_time = current_time;
    }
    
    // 至少两秒内喂狗一次，不然程序复位,只喂五次
    
    if (current_time - feed_last_time >= 1500) {
      
      count++;
      if (count < 5)
      {
        IWDG_ReloadCounter();
      }
      
      feed_last_time = current_time;
      printf("Feed dog\r\n");

      OLED_Printf_Line(1,"Feed dog x %d",count);
    }

    OLED_Refresh_Dirty();

    key = KEY_Get();
    if (key)
    {
      switch (key)
      {
      case KEY2_PRES:

        OLED_Clear();
        return;
      default:
        break;
      }
    }
    
    delay_ms(10);  // 短暂延时，避免过于频繁的查询
  }
}