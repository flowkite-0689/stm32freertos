#include "delay.h"

__IO uint32_t TimingDelay = 0;        // 延时统计多少个10us
__IO uint32_t Systick_count = 0;      // 统计系统运行时间单位ms

// SysTick中断服务函数定义在 stm32f4xx_it.c 中

// 启动系统滴答定时器 SysTick, 10us中断一次
void SysTick_Init(void)
{
    // SystemCoreClock == 168000 000
    // SystemCoreClock / 1000    1ms中断一次
    // SystemCoreClock / 100000  10us中断一次
    // SystemCoreClock / 1000000 1us中断一次
    // 注意：中断一次的时间1us时，整个程序的重心都花在进出中断上了，根本没有时间处理其他的任务，因此不推荐
    if (SysTick_Config(SystemCoreClock / 100000)) {   // 1680
        /* Capture error */
        while (1);
    }
}

// 延时函数单位us (精度10us）
void delay_us(uint32_t us)
{
    TimingDelay = us / 10;    // TimingDelay变量每隔10us在中断服务函数-1
    while (TimingDelay != 0);    // 等待TimingDelay减到0，即延时时间到
}

// 延时函数单位ms
void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        delay_us(1000);
    }
}

// 获取系统运行时间单位ms
uint32_t get_systick(void)
{
    return Systick_count;
}

// 不使用中断的微秒延时(精度1us)
void delay_us_no_irq(uint32_t us)
{
    SysTick->CTRL = 0;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0UL;
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |   // 设置系统定时器的时钟源为内核时钟AHBCLK=168M
                    SysTick_CTRL_ENABLE_Msk;        // 使能定时器;
    // / 等待计数完成
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
    {
    }
    
    //SysTick->CTRL = 0; // 不使用中断方法
    SysTick_Init(); //恢复中断方法    
}

void delay_ms_no_irq(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        delay_us_no_irq(1000);
    }
}

