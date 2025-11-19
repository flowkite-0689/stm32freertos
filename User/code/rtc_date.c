#include "rtc_date.h"
#include <stdio.h>

#define RTC_BKP_DR0_DATA ((uint32_t)0x32F3) // 标记RTC已初始化的标志

// 全局RTC时间结构体定义
RTC_TimeTypeDef g_RTC_Time;
RTC_DateTypeDef g_RTC_Date;

void RTC_Date_Init(void)
{
    // 1) 使能PWR和备份寄存器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    // 2) 允许访问备份寄存器
    PWR_BackupAccessCmd(ENABLE);

    // 3）判断RTC备份寄存器RTC_BKP_DR0中的值是否为RTC_BKP_DR0_DATA，如果不是，则说明RTC未初始化
    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_BKP_DR0_DATA)
    {
        // 4）配置时钟源为LSE
        // 使能LSE时钟(32.768kHz)，并等待LSE时钟稳定
        RCC_LSEConfig(RCC_LSE_ON);
        // 等待LSE稳定
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
            ;
        // 选择LSE作为RTC时钟源
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

        // 5）使能RTC时钟
        RCC_RTCCLKCmd(ENABLE);

        // 等待RTC寄存器同步
        RTC_WaitForSynchro();

        // 6）配置RTC预分频器
        // 设置时间格式为24小时制
        RTC_InitTypeDef RTC_InitStructure;
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; // 24小时制
        RTC_InitStructure.RTC_AsynchPrediv = 127;             // 异步分频，128分频
        RTC_InitStructure.RTC_SynchPrediv = 255;              // 同步分频，256分频
        RTC_Init(&RTC_InitStructure);

        // 7）设置时间
        RTC_TimeTypeDef RTC_TimeStruct;
        RTC_TimeStruct.RTC_H12 = RTC_H12_PM;          // 下午 24小时制可以不写这个参数
        RTC_TimeStruct.RTC_Hours = 10;                // 时
        RTC_TimeStruct.RTC_Minutes = 59;              // 分
        RTC_TimeStruct.RTC_Seconds = 30;              // 秒
        RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct); // RTC_Format_BIN表示二进制格式

        // 8）设置日期
        RTC_DateTypeDef RTC_DateStruct;
        RTC_DateStruct.RTC_Year = 25;                    // 年
        RTC_DateStruct.RTC_Month = 11;                   // 月
        RTC_DateStruct.RTC_Date = 17;                    // 日
        RTC_DateStruct.RTC_WeekDay = RTC_Weekday_Sunday; // 星期
        RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);    // RTC_Format_BIN表示二进制格式

        // 9）标记RTC已初始化
        RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_BKP_DR0_DATA);
    }
    else
    {
        // 等待RTC寄存器同步
        RTC_WaitForSynchro();
    }
}

void RTC_Date_Get(void)
{
    // 1）读取时间并存储到全局变量
    RTC_GetTime(RTC_Format_BIN, &g_RTC_Time);

    // 2）读取日期并存储到全局变量
    RTC_GetDate(RTC_Format_BIN, &g_RTC_Date);

    // 3）输出时间
    printf("Time: %02d:%02d:%02d\n", g_RTC_Time.RTC_Hours, g_RTC_Time.RTC_Minutes, g_RTC_Time.RTC_Seconds);

    // 4）输出日期
    printf("Date: %04d-%02d-%02d\n", g_RTC_Date.RTC_Year + 2000, g_RTC_Date.RTC_Month, g_RTC_Date.RTC_Date);
}
