/**
 * @file delay.c
 * @brief STM32F4xx 延时函数模块实现
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 */

#include "delay.h"

// 保存系统时钟频率，用于计算延时参数
static uint8_t g_sys_clk = 168;

/**
 * @brief 延时函数初始化
 * @param sys_clk 系统时钟频率（单位：MHz）
 * @note 初始化延时参数，根据系统时钟配置延时参数
 */
void delay_init(uint8_t sys_clk)
{
    g_sys_clk = sys_clk;
}

/**
 * @brief 简单的循环延时函数
 * @param count 延时循环次数
 * @note 这是一个简单的软件延时，延时时间取决于CPU频率
 */
void Delay_Cycles(uint32_t count)
{
    while (count--) {
        __NOP();  // 空操作，防止编译器优化
    }
}

/**
 * @brief 毫秒级延时函数
 * @param ms 延时时间（单位：毫秒）
 * @note 基于系统时钟的简单延时，根据实际时钟计算循环次数
 * @warning 此函数为软件延时，精度较低，不适合需要精确计时的场景
 */
void delay_ms(uint32_t ms)
{
    uint32_t i;
    for (i = 0; i < ms; i++) {
        Delay_Cycles(g_sys_clk * 1000 / 4);  // 根据系统时钟计算循环次数，约4个时钟周期一个循环
    }
}

/**
 * @brief 微秒级延时函数
 * @param us 延时时间（单位：微秒）
 * @note 基于系统时钟的简单延时，根据实际时钟计算循环次数
 * @warning 此函数为软件延时，精度较低，不适合需要精确计时的场景
 */
void delay_us(uint32_t us)
{
    uint32_t i;
    for (i = 0; i < us; i++) {
        Delay_Cycles(g_sys_clk / 4);  // 根据系统时钟计算循环次数，约4个时钟周期一个循环
    }
}
