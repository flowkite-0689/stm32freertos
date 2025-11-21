#include "alarm.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include <string.h>

// 全局闹钟数组
Alarm_TypeDef g_alarms[MAX_ALARMS];
uint8_t g_alarm_count = 0;

// 内部函数声明
static void Alarm_RTC_Config(void);

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
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    
    // 使能PWR和备份寄存器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    
    // 允许访问备份域
    PWR_BackupAccessCmd(ENABLE);
    
    // 配置RTC闹钟中断
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    // 使能RTC闹钟中断
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 使能RTC闹钟中断
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
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
 */
void Alarm_Check(void)
{
    // 获取当前时间
    RTC_TimeTypeDef currentTime;
    RTC_GetTime(RTC_Format_BIN, &currentTime);
    
    // 检查每个闹钟
    for (uint8_t i = 0; i < g_alarm_count; i++) {
        if (!g_alarms[i].enabled) {
            continue;
        }
        
        // 检查时间是否匹配
        if (g_alarms[i].hour == currentTime.RTC_Hours &&
            g_alarms[i].minute == currentTime.RTC_Minutes &&
            g_alarms[i].second == currentTime.RTC_Seconds) {
            
            // 触发闹钟(这里只是打印信息，实际应用中可以播放音乐等)
            printf("Alarm triggered! Time: %02d:%02d:%02d\n", 
                   g_alarms[i].hour, g_alarms[i].minute, g_alarms[i].second);
            
            // 如果是一次性闹钟，则禁用它
            if (!g_alarms[i].repeat) {
                Alarm_Disable(i);
            }
        }
    }
}

/**
 * @brief 设置RTC硬件闹钟
 */
void Alarm_SetRTCAlarm(void)
{
    RTC_AlarmTypeDef RTC_AlarmStructure;
    RTC_TimeTypeDef RTC_TimeStruct;
    
    // 查找最早触发的闹钟
    uint8_t earliest_index = 0xFF;
    uint32_t earliest_time = 0xFFFFFFFF;
    
    for (uint8_t i = 0; i < g_alarm_count; i++) {
        if (!g_alarms[i].enabled) {
            continue;
        }
        
        // 计算闹钟时间(简化处理)
        uint32_t alarm_time = g_alarms[i].hour * 3600 + g_alarms[i].minute * 60 + g_alarms[i].second;
        
        if (alarm_time < earliest_time) {
            earliest_time = alarm_time;
            earliest_index = i;
        }
    }
    
    // 如果有有效的闹钟
    if (earliest_index != 0xFF) {
        // 禁用闹钟
        RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
        
        // 设置闹钟时间
        RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
        RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = g_alarms[earliest_index].hour;
        RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = g_alarms[earliest_index].minute;
        RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = g_alarms[earliest_index].second;
        
        RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
        RTC_AlarmStructure.RTC_AlarmDateWeekDay = 1;
        
        RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
        
        // 配置闹钟
        RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
        
        // 使能闹钟
        RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
    } else {
        // 没有闹钟，禁用RTC闹钟
        RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
    }
}