#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

__IO uint32_t TimingDelay = 0;        // 延时统计多少个10us
__IO uint32_t Systick_count = 0;      // 统计系统运行时间单位ms

// SysTick中断服务函数定义在 stm32f4xx_it.c 中

// 启动系统滴答定时器 SysTick, 10us中断一次 - RTOS兼容版本
void SysTick_Init(void)
{
    // 如果FreeRTOS调度器已启动，不需要初始化SysTick
    // 因为FreeRTOS会在vTaskStartScheduler()中自动配置SysTick
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        // 在RTOS环境中，SysTick由FreeRTOS管理，不需要初始化
        return;
    }
    else
    {
        // 调度器未启动时，使用原来的初始化方式
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
}

// 延时函数单位us - RTOS兼容版本
void delay_us(uint32_t us)
{
    // 如果FreeRTOS调度器已启动，使用RTOS兼容的延时方式
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        // 对于微秒级延时，我们可以：
        // 1. 如果延时大于1ms，使用vTaskDelay
        // 2. 对于小于1ms的延时，转换为约等于的tick延时或跳过
        if (us >= 1000)
        {
            vTaskDelay(pdMS_TO_TICKS(us / 1000));
        }
        else
        {
            // 对于小于1ms的延时，在RTOS中通常不需要精确实现
            // 可以使用一个极短的自旋等待，但这会短暂阻塞调度器
            // 建议重新设计代码避免使用微秒级延时
            volatile uint32_t count = us * (SystemCoreClock / 1000000) / 4; // 粗略估算
            while(count--);
        }
    }
    else
    {
        // 调度器未启动时，使用原来的轮询方式
        TimingDelay = us / 10;    // TimingDelay变量每隔10us在中断服务函数-1
        while (TimingDelay != 0);    // 等待TimingDelay减到0，即延时时间到
    }
}

// 延时函数单位ms - RTOS兼容版本
void delay_ms(uint32_t ms)
{
    // 如果FreeRTOS调度器已启动，使用vTaskDelay
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        // FreeRTOS的延时是以tick为单位的，需要转换ms到tick
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
    else
    {
        // 调度器未启动时，使用原来的轮询方式
        for (uint32_t i = 0; i < ms; i++)
        {
            delay_us(1000);
        }
    }
}

// 获取系统运行时间单位ms - RTOS兼容版本
uint32_t get_systick(void)
{
    // 如果FreeRTOS调度器已启动，使用FreeRTOS的tick计数
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        // FreeRTOS的tick通常配置为1ms，所以可以直接使用
        return xTaskGetTickCount();
    }
    else
    {
        // 调度器未启动时，使用原来的Systick_count
        return Systick_count;
    }
}

// 不使用中断的微秒延时 - RTOS兼容版本
void delay_us_no_irq(uint32_t us)
{
    // 如果FreeRTOS调度器已启动，使用RTOS安全的方式
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        // 在RTOS中，我们不应该直接操作SysTick寄存器
        // 因为FreeRTOS依赖SysTick进行任务调度
        
        // 对于微秒级延时，如果延时足够长，转换为任务延时
        if (us >= 1000)
        {
            vTaskDelay(pdMS_TO_TICKS(us / 1000));
        }
        else
        {
            // 对于短延时，使用自旋等待，但会短暂阻塞调度器
            volatile uint32_t count = us * (SystemCoreClock / 1000000) / 4; // 粗略估算
            while(count--);
        }
    }
    else
    {
        // 调度器未启动时，可以使用原来的方式
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
}

// 毫秒延时（无中断版本）- RTOS兼容版本
void delay_ms_no_irq(uint32_t ms)
{
    // 如果FreeRTOS调度器已启动，直接使用RTOS延时
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
    else
    {
        // 调度器未启动时，使用原来的方式
        for (uint32_t i = 0; i < ms; i++)
        {
            delay_us_no_irq(1000);
        }
    }
}