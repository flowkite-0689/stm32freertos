#include "key.h"
#include "oled.h"
#include "oled_print.h"
#include "uart_dma.h"
#include "soft_i2c.h"
#include "logo.h"
#include "ui.h"
#include "rtc_date.h"  // 添加RTC头文件
// 选项的个数
#define options_NUM 4

/**
 * @brief 获取星期名称
 * @param weekday 星期几（1-7，1=星期一）
 * @return 星期名称字符串
 */
static const char* get_weekday_name(u8 weekday)
{
    static const char* weekday_names[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    if (weekday >= 1 && weekday <= 7) {
        return weekday_names[weekday - 1];
    }
    return "---";
}

// 选项的图标
unsigned char *options[] =
		{
				gImage_setting,
				gImage_stopwatch,
				gImage_TandH,
				gImage_flashlight

};

void enter_select(u8 selected)
{
	switch (selected)
	{
	case 0:
		
		break;
	case 1:
 setting();
		break;
	case 2:

		break;
	default:
		break;
	}
}

void menu_Refresh(u8 selected)
{
	u8 left;
	if (selected == 0)
	{
		left = options_NUM - 1;
	}
	else
	{
		left = selected - 1;
	}

	u8 right = ((selected + 1) % options_NUM);

	OLED_ShowPicture(0, 16, 32, 32, options[left], 1);
	OLED_ShowPicture(48, 16, 32, 32, options[selected], 0);
	OLED_ShowPicture(96, 16, 32, 32, options[right], 1);
	OLED_Refresh();
}
// 传入一个当前所选择的菜单项，可由上一次调用返回
u8 menu(u8 cho)
{
  u8 flag_RE=1;
	u8 selected = cho;

	u8 key;
	while (1)
	{

		if (flag_RE)
		{
				OLED_Clear();
	 	menu_Refresh(cho);
	
      flag_RE=0;
		}
		
		if (key = KEY_Get())
		{
			switch (key)
			{
			case KEY0_PRES:
				if (selected == 0)
				{
					selected = options_NUM - 1; // 0→最后一项
				}
				else
				{
					selected--;
				}
				menu_Refresh(selected);

				break;
			case KEY1_PRES:
				selected++;
				selected = selected % options_NUM;
				menu_Refresh(selected);
				break;
			case KEY2_PRES:
				OLED_Clear();
				return selected;
				break;
			case KEY3_PRES:
			  flag_RE=1;
				enter_select(selected); // 进入所选择的菜单项
				break;

			default:
				break;
			}
		}
	}
}

int main()
{

	KEY_Init();
	OLED_Init();
	debug_init();
	u8 key;
	// 主循环
	while (1)
	{
		// 读取RTC时间
		RTC_Date_Get();
		OLED_Printf_Line(0, "%02d/%02d/%02d %s",

										 g_RTC_Date.RTC_Year + 2000,
										 g_RTC_Date.RTC_Month,
										 g_RTC_Date.RTC_Date,
										get_weekday_name(g_RTC_Date.RTC_WeekDay));
		OLED_Printf_Line(1, "%02d:%02d:%02d",
										 g_RTC_Time.RTC_Hours,
										 g_RTC_Time.RTC_Minutes,
										 g_RTC_Time.RTC_Seconds);
		OLED_Refresh_Dirty();
		delay_ms(1000);

		if (key = KEY_Get())
		{
			switch (key)
			{
			case KEY3_PRES:
				menu(0);
				break;

			default:
				break;
			}
		}
	}
}
