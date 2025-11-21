#include "code/key.h"
#include "code/uart_dma.h"
#include <stdio.h>
#include "code/timer_general.h"
#include "code/rtc_date.h"
#include "oled.h"
#include "oled_print.h"
#include "ui/setting.h"
#include "ui/flashlight.h"
#include "ui/alarm_menu.h"
#include "ui/step.h"
#include "ui/alarm.h"

/**
 * @brief 电子手表主函数
 * 
 * 按键功能定义：
 * - KEY0 (PA0): 模式切换/确认
 * - KEY1 (PE2): 增加数值/启动秒表
 * - KEY2 (PE3): 减少数值/暂停秒表
 * - KEY3 (PE4): 重置秒表/返回上级
 */

// 手表模式定义
typedef enum {
    WATCH_MODE_TIME_DISPLAY = 0,  // 时间显示模式
    WATCH_MODE_DATE_DISPLAY,      // 日期显示模式
    WATCH_MODE_SET_TIME,          // 设置时间模式
    WATCH_MODE_SET_DATE,          // 设置日期模式
    WATCH_MODE_STOPWATCH,         // 秒表模式
    WATCH_MODE_COUNT              // 模式总数
} WatchMode;

// 秒表状态结构体
typedef struct {
    uint8_t running;              // 运行状态
    uint32_t start_time;          // 开始时间(ms)
    uint32_t pause_time;          // 暂停累计时间(ms)
    uint32_t elapsed_time;        // 总经过时间(ms)
} StopwatchState;

// 全局变量
WatchMode current_mode = WATCH_MODE_TIME_DISPLAY;
WatchMode last_mode = WATCH_MODE_TIME_DISPLAY;
StopwatchState stopwatch = {0};
uint8_t set_time_step = 0;  // 设置时间时的步骤：0=时，1=分，2=秒
uint8_t set_date_step = 0;  // 设置日期时的步骤：0=年，1=月，2=日
uint8_t temp_hours = 0, temp_minutes = 0, temp_seconds = 0;
uint16_t temp_year = 0, temp_month = 0, temp_day = 0;

// 更新间隔定义(ms)
#define DISPLAY_UPDATE_INTERVAL    1000    // 显示更新间隔
#define STOPWATCH_UPDATE_INTERVAL 100     // 秒表更新间隔

// 时间变量
uint32_t last_display_update = 0;
uint32_t last_stopwatch_update = 0;

/**
 * @brief 显示时间
 */
void Display_Time(void)
{
    RTC_Date_Get();
    OLED_Printf_Line(0, "   DIGITAL WATCH");
    OLED_Printf_Line(1, "   %02d:%02d:%02d", 
                     g_RTC_Time.RTC_Hours,
                     g_RTC_Time.RTC_Minutes, 
                     g_RTC_Time.RTC_Seconds);
    OLED_Printf_Line(2, "     %02d/%02d/%04d", 
                     g_RTC_Date.RTC_Date,
                     g_RTC_Date.RTC_Month, 
                     g_RTC_Date.RTC_Year + 2000);
    OLED_Printf_Line(3, "KEY0:Mode KEY1:+");
}

/**
 * @brief 显示日期
 */
void Display_Date(void)
{
    RTC_Date_Get();
    OLED_Printf_Line(0, "   DATE DISPLAY");
    OLED_Printf_Line(1, "       %02d", g_RTC_Date.RTC_Date);
    OLED_Printf_Line(2, "   %02d / %04d", 
                     g_RTC_Date.RTC_Month, 
                     g_RTC_Date.RTC_Year + 2000);
    
    // 显示星期
    const char* weekdays[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    if(g_RTC_Date.RTC_WeekDay >= 1 && g_RTC_Date.RTC_WeekDay <= 7) {
        OLED_Printf_Line(3, "      %s", weekdays[g_RTC_Date.RTC_WeekDay - 1]);
    } else {
        OLED_Printf_Line(3, "      ---");
    }
}

/**
 * @brief 显示时间设置界面
 */
void Display_Set_Time(void)
{
    OLED_Printf_Line(0, "   SET TIME");
    
    // 根据设置步骤高亮显示当前设置项
    switch(set_time_step) {
        case 0:  // 设置小时
            OLED_Printf_Line(1, "  [%02d]:%02d:%02d", temp_hours, temp_minutes, temp_seconds);
            OLED_Printf_Line(2, "    Set Hours");
            break;
        case 1:  // 设置分钟
            OLED_Printf_Line(1, "   %02d:[%02d]:%02d", temp_hours, temp_minutes, temp_seconds);
            OLED_Printf_Line(2, "   Set Minutes");
            break;
        case 2:  // 设置秒
            OLED_Printf_Line(1, "   %02d:%02d:[%02d]", temp_hours, temp_minutes, temp_seconds);
            OLED_Printf_Line(2, "   Set Seconds");
            break;
    }
    
    OLED_Printf_Line(3, "OK:+/- KEY0:Next");
}

/**
 * @brief 显示日期设置界面
 */
void Display_Set_Date(void)
{
    OLED_Printf_Line(0, "   SET DATE");
    
    // 根据设置步骤高亮显示当前设置项
    switch(set_date_step) {
        case 0:  // 设置年
            OLED_Printf_Line(1, "  [%04d]/%02d/%02d", temp_year + 2000, temp_month, temp_day);
            OLED_Printf_Line(2, "     Set Year");
            break;
        case 1:  // 设置月
            OLED_Printf_Line(1, "   %04d:[%02d]/%02d", temp_year + 2000, temp_month, temp_day);
            OLED_Printf_Line(2, "    Set Month");
            break;
        case 2:  // 设置日
            OLED_Printf_Line(1, "   %04d/%02d:[%02d]", temp_year + 2000, temp_month, temp_day);
            OLED_Printf_Line(2, "      Set Day");
            break;
    }
    
    OLED_Printf_Line(3, "OK:+/- KEY0:Next");
}

/**
 * @brief 显示秒表界面
 */
void Display_Stopwatch(void)
{
    OLED_Printf_Line(0, "   STOPWATCH");
    
    if(stopwatch.running) {
        uint32_t current = get_systick();
        stopwatch.elapsed_time = (current - stopwatch.start_time) + stopwatch.pause_time;
    }
    
    uint32_t total_seconds = stopwatch.elapsed_time / 1000;
    uint32_t minutes = (total_seconds / 60) % 60;
    uint32_t seconds = total_seconds % 60;
    uint32_t milliseconds = stopwatch.elapsed_time % 1000 / 10;
    
    OLED_Printf_Line(1, "   %02lu:%02lu:%02lu", minutes, seconds, milliseconds);
    
    if(stopwatch.running) {
        OLED_Printf_Line(2, "    RUNNING");
        OLED_Printf_Line(3, "KEY2:Pause KEY3:Reset");
    } else {
        OLED_Printf_Line(2, "    PAUSED");
        OLED_Printf_Line(3, "KEY1:Start KEY3:Reset");
    }
}

/**
 * @brief 处理按键输入
 */
void Handle_Keys(void)
{
    uint8_t key = KEY_Get();
    if(key == 0) return;
    
    switch(current_mode) {
        case WATCH_MODE_TIME_DISPLAY:
        case WATCH_MODE_DATE_DISPLAY:
            if(key == KEY0_PRES) {  // 模式切换
                current_mode = (current_mode + 1) % WATCH_MODE_STOPWATCH;
            } else if(key == KEY3_PRES) {  // 快速进入秒表
                current_mode = WATCH_MODE_STOPWATCH;
            }
            break;
            
        case WATCH_MODE_SET_TIME:
            if(key == KEY0_PRES) {  // 下一个设置项
                set_time_step++;
                if(set_time_step >= 3) {
                    // 保存设置的时间
                    RTC_SetTime_Manual(temp_hours, temp_minutes, temp_seconds);
                    set_time_step = 0;
                    current_mode = WATCH_MODE_TIME_DISPLAY;
                }
            } else if(key == KEY1_PRES) {  // 增加
                switch(set_time_step) {
                    case 0: temp_hours = (temp_hours + 1) % 24; break;
                    case 1: temp_minutes = (temp_minutes + 1) % 60; break;
                    case 2: temp_seconds = (temp_seconds + 1) % 60; break;
                }
            } else if(key == KEY2_PRES) {  // 减少
                switch(set_time_step) {
                    case 0: temp_hours = (temp_hours + 23) % 24; break;
                    case 1: temp_minutes = (temp_minutes + 59) % 60; break;
                    case 2: temp_seconds = (temp_seconds + 59) % 60; break;
                }
            }
            break;
            
        case WATCH_MODE_SET_DATE:
            if(key == KEY0_PRES) {  // 下一个设置项
                set_date_step++;
                if(set_date_step >= 3) {
                    // 保存设置的日期
                    RTC_SetDate_Manual(temp_year, temp_month, temp_day, 1);  // 星期暂时设为1
                    set_date_step = 0;
                    current_mode = WATCH_MODE_DATE_DISPLAY;
                }
            } else if(key == KEY1_PRES) {  // 增加
                switch(set_date_step) {
                    case 0: temp_year = (temp_year + 1) % 100; break;
                    case 1: temp_month = (temp_month % 12) + 1; break;
                    case 2: 
                        // 根据月份设置最大天数
                        uint8_t max_day = 31;
                        if(temp_month == 4 || temp_month == 6 || temp_month == 9 || temp_month == 11) {
                            max_day = 30;
                        } else if(temp_month == 2) {
                            max_day = ((temp_year % 4 == 0 && temp_year % 100 != 0) || temp_year % 400 == 0) ? 29 : 28;
                        }
                        temp_day = (temp_day % max_day) + 1; 
                        break;
                }
            } else if(key == KEY2_PRES) {  // 减少
                switch(set_date_step) {
                    case 0: temp_year = (temp_year + 99) % 100; break;
                    case 1: temp_month = (temp_month - 2 + 12) % 12 + 1; break;
                    case 2:
                        // 根据月份设置最大天数
                        uint8_t max_day = 31;
                        if(temp_month == 4 || temp_month == 6 || temp_month == 9 || temp_month == 11) {
                            max_day = 30;
                        } else if(temp_month == 2) {
                            max_day = ((temp_year % 4 == 0 && temp_year % 100 != 0) || temp_year % 400 == 0) ? 29 : 28;
                        }
                        temp_day = (temp_day - 2 + max_day) % max_day + 1;
                        break;
                }
            }
            break;
            
        case WATCH_MODE_STOPWATCH:
            if(key == KEY1_PRES && !stopwatch.running) {  // 启动秒表
                stopwatch.running = 1;
                stopwatch.start_time = get_systick();
            } else if(key == KEY2_PRES && stopwatch.running) {  // 暂停秒表
                stopwatch.running = 0;
                stopwatch.pause_time += (get_systick() - stopwatch.start_time);
            } else if(key == KEY3_PRES) {  // 重置秒表
                stopwatch.running = 0;
                stopwatch.start_time = 0;
                stopwatch.pause_time = 0;
                stopwatch.elapsed_time = 0;
            }
            break;
            
        default:
            break;
    }
    
    // 长按KEY3进入设置模式（简化版，只检测在时间显示模式下KEY3长按）
    if(current_mode == WATCH_MODE_TIME_DISPLAY && key == KEY3_PRES) {
        current_mode = WATCH_MODE_SET_TIME;
        // 加载当前时间到临时变量
        RTC_Date_Get();
        temp_hours = g_RTC_Time.RTC_Hours;
        temp_minutes = g_RTC_Time.RTC_Minutes;
        temp_seconds = g_RTC_Time.RTC_Seconds;
        set_time_step = 0;
    }
}

/**
 * @brief 更新显示内容
 */
void Update_Display(void)
{
    uint32_t current_time = get_systick();
    
    switch(current_mode) {
        case WATCH_MODE_TIME_DISPLAY:
            if(current_time - last_display_update >= DISPLAY_UPDATE_INTERVAL) {
                last_display_update = current_time;
                Display_Time();
            }
            break;
            
        case WATCH_MODE_DATE_DISPLAY:
            if(current_time - last_display_update >= DISPLAY_UPDATE_INTERVAL) {
                last_display_update = current_time;
                Display_Date();
            }
            break;
            
        case WATCH_MODE_SET_TIME:
            Display_Set_Time();
            break;
            
        case WATCH_MODE_SET_DATE:
            Display_Set_Date();
            break;
            
        case WATCH_MODE_STOPWATCH:
            if(current_time - last_stopwatch_update >= STOPWATCH_UPDATE_INTERVAL) {
                last_stopwatch_update = current_time;
                Display_Stopwatch();
            }
            break;
    }
}

/**
  * @brief  主函数
  */
int main(void)
{ 
    u8 key;
    
    // 系统初始化
    delay_init(168);        // 初始化延时函数
    uart_dma_init(115200);  // 初始化串口DMA
    OLED_Init();            // 初始化OLED
    KEY_Init();             // 初始化按键
    RTC_Date_Init();        // 初始化RTC
    //BEEP_Init();          // 初始化蜂鸣器
    
    // 初始化闹钟系统
    Alarms_Init();
    
    printf("System Init OK!\r\n");
    
    // 显示启动画面
    Display_Startup_Screen();
    
    // 主循环
    while(1)
    {
        delay_ms(10);
        
        // 检查是否有闹钟触发
        Alarm_Check();
        
        if((key = KEY_Get()) != 0)
        {
            Handle_Keys();
        }
        
        Update_Display();
        
        // 模式切换时清屏
        if(current_mode != last_mode) {
            OLED_Clear();
            last_mode = current_mode;
        }
        
        // 刷新OLED显示
        OLED_Refresh_Dirty();
    }
}
