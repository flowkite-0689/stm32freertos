#include "frid_test.h"

void frid_test()
{
IWDG_ReloadCounter();
	SysTick_Init();
	debug_init();
  OLED_Clear();
   OLED_Printf_Line(0,"RFID_demo_begin");
    OLED_Refresh_Dirty();
  RFID_demo();

  OLED_Printf_Line(0,"RFID_demo_end");


  OLED_Refresh_Dirty();
  u8 key;
  while (1)
  {
IWDG_ReloadCounter(); // 在每个循环中喂狗，防止看门狗复位
    delay_ms(1000);
		LED0=0;
		delay_ms(1000);
		LED0=1;
IWDG_ReloadCounter(); // 再次喂狗，确保不会超时
		printf("helloworld");
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
  }
}
