#ifndef RTC_DATE_H
#define RTC_DATE_H

#include "stm32f4xx.h"
#include "uart_dma.h"

typedef struct
{
  u8 year;
  u8 mon;
  u8 day;
  
}RTC_Data_TypeDef;

// 全局RTC时间结构体
extern RTC_TimeTypeDef g_RTC_Time;
extern RTC_DateTypeDef g_RTC_Date;

void RTC_Date_Init(void);
void RTC_Date_Get(void);

// RTC时间修改函数
void RTC_SetTime_Manual(uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_SetDate_Manual(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday);
void RTC_SetDateTime_Manual(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                           uint8_t hours, uint8_t minutes, uint8_t seconds);

#endif
