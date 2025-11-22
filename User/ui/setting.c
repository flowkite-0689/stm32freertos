#include "setting.h"
#include "ui/alarm_all.h"

// 设置步骤和临时变量
static u8 set_time_step = 0;
static u8 set_date_step = 0;
static u8 temp_hours = 0;
static u8 temp_minutes = 0;
static u8 temp_seconds = 0;
static u8 temp_year = 0;
static u8 temp_month = 0;
static u8 temp_day = 0;
static u8 temp_weekday = 1;  // 星期设置临时变量

/**
 * @brief 获取月份的最大天数
 * @param year 年份（两位数）
 * @param month 月份（1-12）
 * @return 该月的最大天数
 */
static u8 get_max_days_in_month(u8 year, u8 month)
{
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else if (month == 2) {
        // 判断闰年：能被4整除但不能被100整除，或者能被400整除
        u16 full_year = year + 2000;
        if ((full_year % 4 == 0 && full_year % 100 != 0) || full_year % 400 == 0) {
            return 29;
        } else {
            return 28;
        }
    } else {
        return 31;
    }
}

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



/**
 * @brief 显示时间设置界面
 */
void Display_Set_Time(void)
{
    // 根据设置步骤高亮显示当前设置项
    switch(set_time_step) {
        case 0:  // 设置小时
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "  [%02d]:%02d:%02d", temp_hours, temp_minutes, temp_seconds);
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "    Set Hours");
            break;
        case 1:  // 设置分钟
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %02d:[%02d]:%02d", temp_hours, temp_minutes, temp_seconds);
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Minutes");
            break;
        case 2:  // 设置秒
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %02d:%02d:[%02d]", temp_hours, temp_minutes, temp_seconds);
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Seconds");
            break;
    }
    
   
        OLED_Printf_Line(0, "   SET TIME");
        OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
   
    
    // 刷新OLED显示
    OLED_Refresh_Dirty();
}

/**
 * @brief 显示日期设置界面
 */
void Display_Set_Date(void)
{
    // 根据设置步骤高亮显示当前设置项
    switch(set_date_step) {
        case 0:  // 设置年
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "  [%04d]/%02d/%02d %s ", temp_year + 2000, temp_month, temp_day,get_weekday_name(temp_weekday));
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "     Set Year");
            break;
        case 1:  // 设置月
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %04d:[%02d]/%02d %s", temp_year + 2000, temp_month, temp_day,get_weekday_name(temp_weekday));
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "    Set Month");
            break;
        case 2:  // 设置日
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %04d/%02d:[%02d] %s", temp_year + 2000, temp_month, temp_day,get_weekday_name(temp_weekday));
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "      Set Day");
            break;
        case 3:  // 设置星期
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %04d/%02d/%02d:[%s]", temp_year + 2000, temp_month, temp_day,get_weekday_name(temp_weekday));
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "      Set Week");

            break;
    }

   
        OLED_Printf_Line(0, "   SET DATE");
       
            OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
    
    // 刷新OLED显示
    OLED_Refresh_Dirty();
}

/**
 * @brief 处理时间设置交互
 */
void Process_Set_Time(void)
{
    u8 key;
    
    printf("Entering time setting mode\n");  // 调试信息
    
    // 初始化界面
    OLED_Clear();
    set_time_step = 0;
    RTC_Date_Get();  // 获取当前RTC时间
    temp_hours = g_RTC_Time.RTC_Hours;
    temp_minutes = g_RTC_Time.RTC_Minutes;
    temp_seconds = g_RTC_Time.RTC_Seconds;
    printf("Current time: %02d:%02d:%02d\n", temp_hours, temp_minutes, temp_seconds);  // 调试信息
    Display_Set_Time();
    
    while (1) {
        // 全局闹钟处理 - 在时间设置界面也能处理闹钟
        if (Alarm_GlobalHandler()) {
            delay_ms(10);
            continue; // 如果正在处理闹钟提醒，跳过时间设置循环的其他部分
        }
        
        delay_ms(10);
        if ((key = KEY_Get())!=0) {
            printf("Key pressed: %d\n", key);  // 调试信息
            switch (key) {
                case KEY0_PRES:  // 增加
                    switch(set_time_step) {
                        case 0:  // 小时
                            temp_hours = (temp_hours + 1) % 24;
                            break;
                        case 1:  // 分钟
                            temp_minutes = (temp_minutes + 1) % 60;
                            break;
                        case 2:  // 秒
                            temp_seconds = (temp_seconds + 1) % 60;
                            break;
                    }
                    Display_Set_Time();
                    break;
                    
                case KEY1_PRES:  // 减少
                    switch(set_time_step) {
                        case 0:  // 小时
                            temp_hours = (temp_hours == 0) ? 23 : temp_hours - 1;
                            break;
                        case 1:  // 分钟
                            temp_minutes = (temp_minutes == 0) ? 59 : temp_minutes - 1;
                            break;
                        case 2:  // 秒
                            temp_seconds = (temp_seconds == 0) ? 59 : temp_seconds - 1;
                            break;
                    }
                    Display_Set_Time();
                    break;
                    
                case KEY2_PRES:  // 确认/返回
                    // 保存时间设置
                    printf("Saving time: %02d:%02d:%02d\n", temp_hours, temp_minutes, temp_seconds);
                    RTC_SetTime_Manual(temp_hours, temp_minutes, temp_seconds);
                    set_time_step = 0;
                    return;
                    
                case KEY3_PRES:  // 下一个设置项（循环）
                    set_time_step++;
                    if (set_time_step >= 3) {
                        set_time_step = 0;  // 回到第一项，不是退出
                    }
                    Display_Set_Time();
                    break;
            }
        }
    }
}

/**
 * @brief 处理日期设置交互
 */
void Process_Set_Date(void)
{
    u8 key;
    
    printf("Entering date setting mode\n");  // 调试信息
    
    // 初始化界面
    OLED_Clear();
    set_date_step = 0;
    RTC_Date_Get();  // 获取当前RTC时间
    temp_year = g_RTC_Date.RTC_Year;
    temp_month = g_RTC_Date.RTC_Month;
    temp_day = g_RTC_Date.RTC_Date;
    temp_weekday = g_RTC_Date.RTC_WeekDay;  // 获取当前星期
    if (temp_weekday == 0) temp_weekday = 1;  // 确保星期值有效
    printf("Current date: %04d-%02d-%02d (Week: %s)\n", temp_year + 2000, temp_month, temp_day, get_weekday_name(temp_weekday));  // 调试信息
    Display_Set_Date();
    
    while (1) {
        // 全局闹钟处理 - 在日期设置界面也能处理闹钟
        if (Alarm_GlobalHandler()) {
            delay_ms(10);
            continue; // 如果正在处理闹钟提醒，跳过日期设置循环的其他部分
        }
        
        delay_ms(10);
        if ((key = KEY_Get())!=0) {
            printf("Key pressed: %d\n", key);  // 调试信息
            switch (key) {
                case KEY0_PRES:  // 增加
                    switch(set_date_step) {
                        case 0:  // 年
                            temp_year = (temp_year + 1) % 100;
                            // 如果当前日期在新年份的2月29日后，且新年份不是闰年，调整日期
                            if (temp_month == 2 && temp_day == 29) {
                                u8 max_days = get_max_days_in_month(temp_year, temp_month);
                                if (temp_day > max_days) {
                                    temp_day = max_days;
                                }
                            }
                            break;
                        case 1:  // 月
                            temp_month++;
                            if (temp_month > 12) temp_month = 1;
                            // 调整日期，确保不超过新月份的最大天数
                            {
                                u8 max_days = get_max_days_in_month(temp_year, temp_month);
                                if (temp_day > max_days) {
                                    temp_day = max_days;
                                }
                            }
                            break;
                        case 2:  // 日
                            {
                                u8 max_days = get_max_days_in_month(temp_year, temp_month);
                                temp_day++;
                                if (temp_day > max_days) temp_day = 1;
                            }
                            break;
                        case 3:  // 星期
                            temp_weekday++;
                            if (temp_weekday > 7) temp_weekday = 1;
                            break;
                    }
                    Display_Set_Date();
                    break;
                    
                case KEY1_PRES:  // 减少
                    switch(set_date_step) {
                        case 0:  // 年
                            temp_year = (temp_year == 0) ? 99 : temp_year - 1;
                            // 如果当前日期在新年份的2月29日后，且新年份不是闰年，调整日期
                            if (temp_month == 2 && temp_day == 29) {
                                u8 max_days = get_max_days_in_month(temp_year, temp_month);
                                if (temp_day > max_days) {
                                    temp_day = max_days;
                                }
                            }
                            break;
                        case 1:  // 月
                            temp_month--;
                            if (temp_month == 0) temp_month = 12;
                            // 调整日期，确保不超过新月份的最大天数
                            {
                                u8 max_days = get_max_days_in_month(temp_year, temp_month);
                                if (temp_day > max_days) {
                                    temp_day = max_days;
                                }
                            }
                            break;
                        case 2:  // 日
                            {
                                u8 max_days = get_max_days_in_month(temp_year, temp_month);
                                temp_day--;
                                if (temp_day == 0) temp_day = max_days;
                            }
                            break;
                        case 3:  // 星期
                            temp_weekday--;
                            if (temp_weekday == 0) temp_weekday = 7;
                            break;
                    }
                    Display_Set_Date();
                    break;
                    
                case KEY2_PRES:  // 确认/返回
                    // 保存日期设置
                    printf("Saving date: %04d-%02d-%02d (Week: %s)\n", temp_year + 2000, temp_month, temp_day, get_weekday_name(temp_weekday));
                    RTC_SetDate_Manual(temp_year, temp_month, temp_day, temp_weekday);
                    set_date_step = 0;
                    return;
                    
                case KEY3_PRES:  // 下一个设置项（循环）
                    set_date_step++;
                    if (set_date_step >= 4) {
                        set_date_step = 0;  // 回到第一项，不是退出
                    }
                    Display_Set_Date();
                    break;
            }
        }
    }
}



// 选项的图标
const unsigned char *set_options[] =
    {
        gImage_clock,
        gImage_calendar

};
void set_Enter_select(u8 selected)
{
  switch (selected)
  {
  case 0:
    Process_Set_Time();  // 进入时间设置交互
    break;
  case 1:
    Process_Set_Date();  // 进入日期设置交互
    break;

  default:
    break;
  }
}
void setting_Ref(u8 selected)
{
  u8 right = ((selected + 1) % set_options_NUM);

  OLED_ShowPicture(48, 16, 32, 32, set_options[selected], 0);
  OLED_ShowPicture(96, 16, 32, 32, set_options[right], 1);
  OLED_Refresh();
}

void setting()
{

  u8 flag_RE=1;
  u8 key;
  u8 selected= 0;
 
  while (1)
  {
    // 全局闹钟处理 - 在设置界面也能处理闹钟
    if (Alarm_GlobalHandler()) {
        delay_ms(10);
        continue; // 如果正在处理闹钟提醒，跳过设置循环的其他部分
    }
    
    delay_ms(10);
    if (flag_RE)
    {
       OLED_Clear();
       setting_Ref(selected);
       flag_RE=0;
    }
    
    if ((key = KEY_Get())!=0)
    {
      switch (key)
      {
      case KEY0_PRES:
        if (selected == 0)
        {
          selected = set_options_NUM - 1; // 0→最后一项
        }
        else
        {
          selected--;
        }
        setting_Ref(selected);

        break;
      case KEY1_PRES:
        selected++;
        selected = selected % set_options_NUM;
        setting_Ref(selected);
        break;
      case KEY2_PRES:
        OLED_Clear();
        return ;
       
      case KEY3_PRES:
      flag_RE=1;
        set_Enter_select(selected); // 进入所选择的菜单项
        break;

      default:
        break;
      }
    }
  }
}
