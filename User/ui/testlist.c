#include "testlist.h"
#define SHOWING_NUM 4

char *test_opt[] = {
    "SPI_test",
    "2048_oled",
    "frid_test",
    // "filesystem_test",
    "iwdg_test",
   "air_level"};

#define TOTAL_ITEMS (sizeof(test_opt) / sizeof(test_opt[0]))

void SPI_test_Re()
{
  OLED_Clear();
  OLED_Printf_Line(1, "SPI_TEST");
  OLED_Refresh();
}

void SPI_test()
{
  SPI_test_Re();
  SPI1_Init();
  u32 id = W25Q128_ReadID();
  if (id == W25X_JEDECID)
  {
    printf("读到的ID正确:%#x\n", id);
    OLED_Printf_Line(2, "OK:%#x\n", id);
  }
  else
  {
    printf("读到的ID失败:%#x\n", id);
    OLED_Printf_Line(2, "ERR:%#x\n", id);
  }
  OLED_Refresh_Dirty();
  u8 key;
  while (1)
  {IWDG_ReloadCounter();
    delay_ms(10);
    key = KEY_Get();
    if (key)
    {
      switch (key)
      {
      case KEY2_PRES:
        return;

      default:
        break;
      }
    }
  }
}

void test_enter_select(u8 selected)
{
  switch (selected)
  {
  case 0:
    SPI_test();
    break;
  case 1:
    menu_2048_oled();
    break;
    case 2:
    frid_test();
    break;
  // case 3:
  //   filesystem_test();
  //   break;
  case 3:
    iwdg_test();
    break;
    case 4: 
    air_level_test();
    break;
  default:
    break;
  }
}
u8 last_page = 0;
// 安全的菜单显示函数
void tsetlist_RE(u8 selected)
{
  u8 page = selected / SHOWING_NUM;
  if (last_page != page)
  {
    OLED_Clear();
    last_page = page;
  }
  // 当前在第几页（从0开始）
  u8 start_idx = page * SHOWING_NUM;            // 本页第一个选项的索引
  u8 items_this_page = TOTAL_ITEMS - start_idx; // 本页实际有多少项
  if (items_this_page > SHOWING_NUM)
    items_this_page = SHOWING_NUM;

  for (u8 i = 0; i < items_this_page; i++)
  {
    u8 current_idx = start_idx + i; // 实际在总列表中的索引

    // 判断是否是当前选中项
    char arrow = (current_idx == selected) ? '>' : ' ';

    // 显示这一行（假设第0行开始显示菜单）
    OLED_Printf_Line(i, "%c %s", arrow, test_opt[current_idx]);
  }

  // 如果本页不足4行，下面几行可以清空或留空
  for (u8 i = items_this_page; i < SHOWING_NUM; i++)
  {
    OLED_Clear_Line(i); // 8个空格清行
  }

  OLED_Refresh_Dirty();
}

// 测试功能的列表
void testlist()
{
  u8 flag_RE = 1;
  u8 selected = 0;
  tsetlist_RE(selected);
  u32 current_time = get_systick();
  u8 key;
  while (1)
  {IWDG_ReloadCounter();
    delay_ms(10);
    if (flag_RE)
    {
      OLED_Clear();
      tsetlist_RE(selected);
      current_time = get_systick();
      flag_RE = 0;
    }

    if ((key = KEY_Get()) != 0)
    {
      if (get_systick() - current_time > 500)
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
          return;

        case KEY3_PRES:
          flag_RE = 1;
          OLED_Clear();
          test_enter_select(selected); // ???????????????
          break;

        default:
          break;
        }
      }
    }
  }
}
