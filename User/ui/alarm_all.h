#ifndef _ALARM_ALL_H
#define _ALARM_ALL_H

#include "stm32f4xx.h"
#include "rtc_date.h"
#include <stdint.h>

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
extern Alarm_TypeDef g_alarms[MAX_ALARMS];
extern uint8_t g_alarm_count;
extern uint8_t g_triggered_alarm_index;

// 闹钟提醒状态
extern uint8_t alarm_alert_active;

// 闹钟系统函数
void Alarms_Init(void);
void Alarms_Save(void);
void Alarms_Load(void);
uint8_t Alarm_Add(Alarm_TypeDef* alarm);
void Alarm_Delete(uint8_t index);
void Alarm_Enable(uint8_t index);
void Alarm_Disable(uint8_t index);
void Alarm_Check(void);
void Alarm_SetRTCAlarm(void);

// 闹钟提醒函数
void Display_Alarm_Alert(Alarm_TypeDef* alarm);
uint8_t Handle_Alarm_Alert_Keys(void);
void Update_Alarm_Alert_Display(Alarm_TypeDef* alarm);

// 全局闹钟处理函数(没有实现)
uint8_t Alarm_GlobalHandler(void);

// 测试函数
void Alarm_ForceTrigger(void);

#endif
