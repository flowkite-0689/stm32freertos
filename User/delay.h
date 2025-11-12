/**
 * @file delay.h
 * @brief STM32F4xx 延时函数模块
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 * 
 * 本文件提供STM32F4系列微控制器的延时功能，
 * 包括毫秒级延时和微秒级延时。
 * 采用分层架构设计，提供精确的软件延时接口。
 */

#ifndef _DELAY_H_
#define _DELAY_H_

#include "stm32f4xx.h"

// ==================================
// 延时函数声明
// ==================================

/**
 * @brief 延时函数初始化
 * @param sys_clk 系统时钟频率（单位：MHz）
 * @note 初始化延时参数，根据系统时钟配置延时参数
 */
void delay_init(uint8_t sys_clk);

/**
 * @brief 简单的循环延时函数
 * @param count 延时循环次数
 * @note 这是一个简单的软件延时，延时时间取决于CPU频率
 */
void Delay_Cycles(uint32_t count);

/**
 * @brief 毫秒级延时函数
 * @param ms 延时时间（单位：毫秒）
 * @note 基于系统时钟的简单延时，根据实际时钟计算循环次数
 * @warning 此函数为软件延时，精度较低，不适合需要精确计时的场景
 */
void delay_ms(uint32_t ms);

/**
 * @brief 微秒级延时函数
 * @param us 延时时间（单位：微秒）
 * @note 基于系统时钟的简单延时，根据实际时钟计算循环次数
 * @warning 此函数为软件延时，精度较低，不适合需要精确计时的场景
 */
void delay_us(uint32_t us);



void SysTick_Init(void);
uint32_t get_systick(void);

#endif /* _DELAY_H_ */
