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
    // printf("Time: %02d:%02d:%02d\n", g_RTC_Time.RTC_Hours, g_RTC_Time.RTC_Minutes, g_RTC_Time.RTC_Seconds);

    // 4）输出日期
    // printf("Date: %04d-%02d-%02d\n", g_RTC_Date.RTC_Year + 2000, g_RTC_Date.RTC_Month, g_RTC_Date.RTC_Date);
}

// 手动设置RTC时间
void RTC_SetTime_Manual(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    // 参数检查
    if (hours > 23) hours = 23;
    if (minutes > 59) minutes = 59;
    if (seconds > 59) seconds = 59;
    
    // 解除RTC写保护
    PWR_BackupAccessCmd(ENABLE);
    
    // 进入初始化模式
    RTC_WriteProtectionCmd(DISABLE);
    
    // 设置时间结构体
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_TimeStruct.RTC_H12 = RTC_H12_AM;          // 24小时制，设置为AM
    RTC_TimeStruct.RTC_Hours = hours;
    RTC_TimeStruct.RTC_Minutes = minutes;
    RTC_TimeStruct.RTC_Seconds = seconds;
    
    // 设置RTC时间
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
    
    // 重新启用写保护
    RTC_WriteProtectionCmd(ENABLE);
    
    printf("RTC Time set to: %02d:%02d:%02d\n", hours, minutes, seconds);
}

// 手动设置RTC日期
void RTC_SetDate_Manual(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday)
{
    // 参数检查
    if (year > 99) year = 99;
    if (month < 1) month = 1;
    if (month > 12) month = 12;
    if (day < 1) day = 1;
    if (day > 31) day = 31;
    if (weekday < 1) weekday = 1;
    if (weekday > 7) weekday = 7;
    
    // 解除RTC写保护
    PWR_BackupAccessCmd(ENABLE);
    
    // 进入初始化模式
    RTC_WriteProtectionCmd(DISABLE);
    
    // 设置日期结构体
    RTC_DateTypeDef RTC_DateStruct;
    RTC_DateStruct.RTC_Year = year;
    RTC_DateStruct.RTC_Month = month;
    RTC_DateStruct.RTC_Date = day;
    RTC_DateStruct.RTC_WeekDay = weekday;
    
    // 设置RTC日期
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
    
    // 重新启用写保护
    RTC_WriteProtectionCmd(ENABLE);
    
    printf("RTC Date set to: %04d-%02d-%02d (Weekday: %d)\n", year + 2000, month, day, weekday);
}

// 手动设置完整的RTC日期时间
void RTC_SetDateTime_Manual(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                           uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    // 先设置日期
    RTC_SetDate_Manual(year, month, day, weekday);
    
    // 再设置时间
    RTC_SetTime_Manual(hours, minutes, seconds);
    
    printf("RTC DateTime set complete!\n");
}
