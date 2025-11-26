#ifndef DELAY_H
#define DELAY_H
#include "stm32f4xx.h"

// 声明全局变量供其他文件使用
extern __IO uint32_t TimingDelay;
extern __IO uint32_t Systick_count;

void SysTick_Init(void);
void TIM6_Delay_Init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
uint32_t get_systick(void);
void delay_us_no_irq(uint32_t us);
void delay_ms_no_irq(uint32_t ms);

#endif
