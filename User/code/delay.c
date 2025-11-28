#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

__IO uint32_t TimingDelay = 0;        // 延时统计多少个10us
__IO uint32_t Systick_count = 0;      // 统计系统运行时间单位ms

// TIM6 微秒延时初始化
static void TIM6_Delay_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    // 使能TIM6时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    
    // 配置TIM6为1MHz计数频率（1us一个计数）
    // TIM6时钟来源：APB1 = 84MHz (当APB1预分频不为1时，定时器时钟x2)
    // 所以TIM6实际时钟为84MHz
    TIM_TimeBaseStructure.TIM_Period = 0xFFFFFFFF;       // 最大计数值
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;       // 84MHz/84 = 1MHz (1us计数一次)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;       // 时钟分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
    
    // 不使能定时器，在需要延时的时候再启动
    TIM_Cmd(TIM6, DISABLE);
}

// SysTick中断服务函数定义在 stm32f4xx_it.c 中

void SysTick_Init(void)
{
    // 如果FreeRTOS调度器已启动，不需要初始化SysTick
    // 因为FreeRTOS会在vTaskStartScheduler()中自动配置SysTick
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        return;
    }
    else
    {
        // 调度器未启动时，使用原来的初始化方式
        // SystemCoreClock == 168000 000
        // SystemCoreClock / 1000    1ms中断一次
        // SystemCoreClock / 100000  10us中断一次
        // SystemCoreClock / 1000000 1us中断一次
        if (SysTick_Config(SystemCoreClock / 100000)) {   // 1680
            /* Capture error */
            while (1);
        }
    }
}


// 改为基本定时器
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
        // FreeRTOS的tick通常配置为1ms，
        return xTaskGetTickCount();
    }
    else
    {
        // 调度器未启动时，使用原来的Systick_count
        return Systick_count;
    }
}

// 不使用中断的微秒延时 
void delay_us_no_irq(uint32_t us)
{
    static uint8_t tim6_initialized = 0;
    
    // 
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        // 对于微秒级延时，如果延时足够长，转换为任务延时
        if (us >= 1000)
        {
            vTaskDelay(pdMS_TO_TICKS(us / 1000));
        }
        else
        {
            // 对于短延时，使用TIM6定时器实现精确延时
            if (!tim6_initialized)
            {
                TIM6_Delay_Init();
                tim6_initialized = 1;
            }
            
            // 设置计数器为0
            TIM_SetCounter(TIM6, 0);
            
            // 启动TIM6
            TIM_Cmd(TIM6, ENABLE);
            
            // 等待计数达到指定值
            while (TIM_GetCounter(TIM6) < us)
            {
                // 空循环等待
            }
            
            // 停止TIM6
            TIM_Cmd(TIM6, DISABLE);
        }
    }
    else
    {
        // 调度器未启动时，也使用TIM6实现精确延时
        if (!tim6_initialized)
        {
            TIM6_Delay_Init();
            tim6_initialized = 1;
        }
        
        // 设置计数器为0
        TIM_SetCounter(TIM6, 0);
        
        // 启动TIM6
        TIM_Cmd(TIM6, ENABLE);
        
        // 等待计数达到指定值
        while (TIM_GetCounter(TIM6) < us)
        {
            // 空循环等待
        }
        
        // 停止TIM6
        TIM_Cmd(TIM6, DISABLE);
    }
}

// 毫秒延时
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
