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

#endif
