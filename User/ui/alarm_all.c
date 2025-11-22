#include "stm32f4xx.h"
#include "rtc_date.h"
#include "oled.h"
#include "oled_print.h"
#include "key.h"
#include "logo.h"
#include "code/led.h"
#include "code/delay.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include <string.h>
#include <stdio.h>

// 最大闹钟数量
#define MAX_ALARMS 10

// 闹钟结构体
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t enabled;
    uint8_t repeat;         // 0=一次性, 1=每天重复
    uint8_t daysOfWeek;     // 位标志：bit0=周一 ... bit6=周日
} Alarm_TypeDef;

// 全局闹钟数组
Alarm_TypeDef g_alarms[MAX_ALARMS];
uint8_t g_alarm_count = 0;
// 当前触发的闹钟索引
uint8_t g_triggered_alarm_index = 0xFF;

// 闹钟提醒状态
uint8_t alarm_alert_active = 0;

// 内部函数声明
static void Alarm_RTC_Config(void);
static void Alarm_SetRTCAlarm(void);
static void Alarms_Load(void);
static void Display_Alarm_Alert(Alarm_TypeDef* alarm);
static void Update_Alarm_Alert_Display(Alarm_TypeDef* alarm);

// 全局函数声明（在头文件中已有，但这里重复声明以避免编译器警告）
extern uint32_t get_systick(void);

/**
 * @brief 初始化闹钟系统
 */
void Alarms_Init(void)
{
    // 初始化闹钟数组
    memset(g_alarms, 0, sizeof(g_alarms));
    g_alarm_count = 0;
    
    // 加载已保存的闹钟
    Alarms_Load();
    
    // 配置RTC闹钟
    Alarm_RTC_Config();
}

/**
 * @brief 配置RTC闹钟
 */
static void Alarm_RTC_Config(void)
{
    // 使能PWR和备份寄存器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    
    // 允许访问备份域
    PWR_BackupAccessCmd(ENABLE);
    
    // 暂时不配置RTC闹钟中断，改用纯软件检查
    // 因为硬件闹钟可能干扰RTC时间更新
    
    printf("RTC Alarm config initialized (software mode)\r\n");
}

/**
 * @brief 保存闹钟到备份寄存器(简化版，实际应用中可能需要使用Flash或EEPROM)
 */
void Alarms_Save(void)
{
    // 这里只是一个示例，实际应用中应该将闹钟数据保存到Flash或EEPROM中
    // 当前版本仅在运行时保存在内存中
}

/**
 * @brief 从备份寄存器加载闹钟(简化版)
 */
void Alarms_Load(void)
{
    // 这里只是一个示例，实际应用中应该从Flash或EEPROM中加载闹钟数据
    // 当前版本仅在运行时保存在内存中
}

/**
 * @brief 添加新闹钟
 * @param alarm 指向闹钟结构体的指针
 * @return 0成功，其他值失败
 */
uint8_t Alarm_Add(Alarm_TypeDef* alarm)
{
    if (g_alarm_count >= MAX_ALARMS) {
        return 1; // 闹钟数量已达上限
    }
    
    g_alarms[g_alarm_count] = *alarm;
    g_alarm_count++;
    
    // 保存闹钟
    Alarms_Save();
    
    // 更新RTC闹钟
    Alarm_SetRTCAlarm();
    
    return 0;
}

/**
 * @brief 删除指定索引的闹钟
 * @param index 闹钟索引
 */
void Alarm_Delete(uint8_t index)
{
    if (index >= g_alarm_count) {
        return;
    }
    
    // 将后面的元素向前移动
    for (uint8_t i = index; i < g_alarm_count - 1; i++) {
        g_alarms[i] = g_alarms[i + 1];
    }
    
    g_alarm_count--;
    
    // 保存闹钟
    Alarms_Save();
    
    // 更新RTC闹钟
    Alarm_SetRTCAlarm();
}

/**
 * @brief 启用指定索引的闹钟
 * @param index 闹钟索引
 */
void Alarm_Enable(uint8_t index)
{
    if (index >= g_alarm_count) {
        return;
    }
    
    g_alarms[index].enabled = 1;
    
    // 保存闹钟
    Alarms_Save();
    
    // 更新RTC闹钟
    Alarm_SetRTCAlarm();
}

/**
 * @brief 禁用指定索引的闹钟
 * @param index 闹钟索引
 */
void Alarm_Disable(uint8_t index)
{
    if (index >= g_alarm_count) {
        return;
    }
    
    g_alarms[index].enabled = 0;
    
    // 保存闹钟
    Alarms_Save();
    
    // 更新RTC闹钟
    Alarm_SetRTCAlarm();
}

/**
 * @brief 检查当前时间是否有闹钟触发
 * 纯软件检查，避免RTC闹钟配置影响RTC时间更新
 */
void Alarm_Check(void)
{
    static uint8_t last_second = 99; // 初始值设为不可能的值
    
    // 如果已经有闹钟在提醒，不再检查新的闹钟
    if (alarm_alert_active) {
        return;
    }
    
    // 获取当前时间
    RTC_TimeTypeDef currentTime;
    RTC_GetTime(RTC_Format_BIN, &currentTime);
    
    // 调试：每秒输出一次时间
    // if (currentTime.RTC_Seconds != last_second) {
    //     printf("Time: %02d:%02d:%02d\r\n", 
    //            currentTime.RTC_Hours, 
    //            currentTime.RTC_Minutes, 
    //            currentTime.RTC_Seconds);
    //     last_second = currentTime.RTC_Seconds;
    // }
    
    // 检查每个闹钟
    for (uint8_t i = 0; i < g_alarm_count; i++) {
        if (!g_alarms[i].enabled) {
            continue;
        }
        
        // 检查时间是否完全匹配
        if (g_alarms[i].hour == currentTime.RTC_Hours &&
            g_alarms[i].minute == currentTime.RTC_Minutes &&
            g_alarms[i].second == currentTime.RTC_Seconds) {
            
            // 触发闹钟
            printf("Alarm triggered! Time: %02d:%02d:%02d, Index: %d\n", 
                   currentTime.RTC_Hours, currentTime.RTC_Minutes, currentTime.RTC_Seconds, i);
            
            // 点亮LED2
            LED_Set(2, 0);  // 点亮LED2
            
            // 设置闹钟提醒状态为激活
            alarm_alert_active = 1;
            // 记录触发的闹钟索引
            g_triggered_alarm_index = i;
            
            // 如果是一次性闹钟，则禁用它
            if (!g_alarms[i].repeat) {
                g_alarms[i].enabled = 0;
            }
            
            // 一旦找到匹配的闹钟就退出循环
            break;
        }
    }
}

/**
 * @brief 设置RTC硬件闹钟
 * 暂时简化为不使用硬件闹钟，避免RTC时间更新问题
 */
void Alarm_SetRTCAlarm(void)
{
    // 暂时禁用硬件RTC闹钟，完全使用软件检查
    // 这样可以避免RTC闹钟配置影响RTC时间更新
    
    for (uint8_t i = 0; i < g_alarm_count; i++) {
        if (g_alarms[i].enabled) {
            printf("Software alarm set for %02d:%02d:%02d\r\n", 
                   g_alarms[i].hour, g_alarms[i].minute, g_alarms[i].second);
        }
    }
}

/**
 * @brief 显示闹钟提醒界面
 */
void Display_Alarm_Alert(Alarm_TypeDef* alarm)
{
    printf("=== Display_Alarm_Alert called ===\r\n");
    
    OLED_Clear(); // 完全清除屏幕，而不是只清除几行
    
    // 显示闹钟图标
    OLED_ShowPicture(48, 0, 32, 32, gImage_bell, 0);
    
    // 显示提醒文字
    OLED_Printf_Line(0, "    ALARM!");
    OLED_Printf_Line(2, "  %02d:%02d:%02d", alarm->hour, alarm->minute, alarm->second);
    OLED_Printf_Line(3, "Press KEY3 to stop");
    
    printf("Displaying alarm alert for %02d:%02d:%02d\r\n", 
           alarm->hour, alarm->minute, alarm->second);
    
    OLED_Refresh();
    
    // 强制刷新显示
    delay_ms(10);
    OLED_Refresh_Dirty();
    
    printf("=== Alarm display completed ===\r\n");
}

/**
 * @brief 处理闹钟提醒界面的按键输入
 * @return 1表示需要退出提醒界面，0表示继续留在提醒界面
 */
uint8_t Handle_Alarm_Alert_Keys(void)
{
    uint8_t key = KEY_Get();
    
    // 只有KEY3可以关闭闹钟提醒
    if (key == KEY3_PRES) {
        // 关闭LED2
        LED_Set(2, 1);  // 熄灭LED2
        return 1;  // 返回主界面
    }
    
    return 0;  // 继续留在提醒界面
}

/**
 * @brief 更新闹钟提醒显示
 */
void Update_Alarm_Alert_Display(Alarm_TypeDef* alarm)
{
    // 更新显示（可添加动画效果等）
    Display_Alarm_Alert(alarm);
}

/**
 * @brief 强制触发测试闹钟
 * 用于测试闹钟显示和按键响应
 */
void Alarm_ForceTrigger(void)
{
    printf("Force triggering alarm test...\r\n");
    
    // 点亮LED2
    LED_Set(2, 0);  
    
    // 设置闹钟提醒状态为激活
    alarm_alert_active = 1;
    g_triggered_alarm_index = 0xFF; // 特殊值表示测试闹钟
    
    // 创建默认测试闹钟并直接显示
    static Alarm_TypeDef test_alarm = {
        .hour = 12, .minute = 0, .second = 0,
        .enabled = 1, .repeat = 0, .daysOfWeek = 0
    };
    Display_Alarm_Alert(&test_alarm);
    printf("Test alarm display activated\r\n");
    
    // 强制刷新显示
    delay_ms(10);
    OLED_Refresh_Dirty();
}

/**
 * @brief 全局闹钟状态处理函数
 * 任何界面都可以调用此函数来检查和处理闹钟状态
 * @return 1表示当前正在处理闹钟提醒，0表示无闹钟提醒
 */
uint8_t Alarm_GlobalHandler(void)
{
    // 如果处于闹钟提醒状态
    if (alarm_alert_active) {
        // 更新闹钟提醒显示
        if (g_triggered_alarm_index != 0xFF && g_triggered_alarm_index < g_alarm_count) {
            Update_Alarm_Alert_Display(&g_alarms[g_triggered_alarm_index]);
            printf("Updating alarm display for index %d\r\n", g_triggered_alarm_index);
        } else if (g_triggered_alarm_index == 0xFF) {
            // 这是测试闹钟，创建默认显示
            static Alarm_TypeDef test_alarm = {
                .hour = 0, .minute = 0, .second = 0,
                .enabled = 1, .repeat = 0, .daysOfWeek = 0
            };
            printf("Updating test alarm display\r\n");
            Display_Alarm_Alert(&test_alarm);
        }
        
        // 处理闹钟提醒界面的按键输入
        uint8_t key = KEY_Get();
        if (key == KEY3_PRES) {
            printf("KEY3 pressed - dismissing alarm\r\n");
            // 关闭LED2
            LED_Set(2, 1);  // 熄灭LED2
            alarm_alert_active = 0;  // 退出提醒状态
            g_triggered_alarm_index = 0xFF; // 重置触发索引
            
            OLED_Clear(); // 清除显示，返回原界面
            printf("Alarm dismissed, returning to normal mode\r\n");
            return 0; // 闹钟处理完毕
        } else if (key != 0) {
            printf("Other key pressed: %d\r\n", key);
        }
        
        return 1; // 仍在处理闹钟提醒
    }
    
    return 0; // 无闹钟提醒
}
