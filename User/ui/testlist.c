#include "testlist.h"
#define SHOWING_NUM 4

char *test_opt[] = {
    "SPI_test",
    "MPU6050",
    "light_test",
    "test_1",
    "t2",
    "t3"
};

#define TOTAL_ITEMS (sizeof(test_opt)/sizeof(test_opt[0]))

void SPI_test()
{

}

void test_enter_select(u8 selected)
{
  switch (selected)
  {
  case 0:
    SPI_test();
    break;
  
  default:
    break;
  }
}
u8 last_page=0;
// 安全的菜单显示函数
void tsetlist_RE(u8 selected)
{
    u8 page = selected / SHOWING_NUM;   
    if (last_page!=page)
    {
      OLED_Clear();
      last_page =page;
    }
                    // 当前在第几页（从0开始）
    u8 start_idx = page * SHOWING_NUM;                  // 本页第一个选项的索引
    u8 items_this_page = TOTAL_ITEMS - start_idx;       // 本页实际有多少项
    if (items_this_page > SHOWING_NUM) 
        items_this_page = SHOWING_NUM;

    for (u8 i = 0; i < items_this_page; i++)
    {
        u8 current_idx = start_idx + i;                 // 实际在总列表中的索引
        
        // 判断是否是当前选中项
        char arrow = (current_idx == selected) ? '>' : ' ';
        
        // 显示这一行（假设第0行开始显示菜单）
        OLED_Printf_Line(i, "%c %s", arrow, test_opt[current_idx]);
    }
    
    // 如果本页不足4行，下面几行可以清空或留空
    for (u8 i = items_this_page; i < SHOWING_NUM; i++)
    {
      OLED_Clear_Line(i);  // 8个空格清行
    }

OLED_Refresh_Dirty();

  
}

// 测试功能的列表
void testlist()
{
  u8 flag_RE=1;
  u8 selected=0;
  tsetlist_RE(selected);
  u8 key;
  while (1)
  {
    delay_ms(10);
    if (flag_RE)
    {
      OLED_Clear();
       tsetlist_RE(selected);
       flag_RE=0;
    }
    
    if (( key = KEY_Get())!=0 )
    {
      switch (key)
			{
			case KEY0_PRES:
				if (selected == 0)
				{
					selected = TOTAL_ITEMS - 1; // 0?????????
				}
				else
				{
					selected--;
				}
				tsetlist_RE(selected);

				break;
			case KEY1_PRES:
				selected++;
				selected = selected % TOTAL_ITEMS;
				tsetlist_RE(selected);
				break;
			case KEY2_PRES:
				OLED_Clear();
				return ;

			case KEY3_PRES:
				flag_RE = 1;
				test_enter_select(selected); // ???????????????
				break;

			default:
				break;
			}
    }
    
  }
  
}
