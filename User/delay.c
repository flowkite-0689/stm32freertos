/**
 * @file delay.c
 * @brief STM32F4xx 延时函数模块实现
 * @author flowkite-0689
 * @version v2.1
 * @date 2025.11.12
 * 
 * 本模块提供基于SysTick定时器的精确延时功能，包括：
 * - 微秒级延时 (delay_us)
 * - 毫秒级延时 (delay_ms)
 * - 系统时钟获取 (get_systick)
 * - SysTick初始化 (SysTick_Init)
 */

#include "delay.h"

/* 全局变量定义 ---------------------------------------------------------------*/
__IO uint32_t TimingDelay = 0;    /**< 延时计数器，用于delay_us()函数的延时控制 */
__IO uint32_t Systick_count = 0;  /**< 系统时钟计数器，记录自启动以来的毫秒数 */

/* 私有函数声明 ---------------------------------------------------------------*/

/* 函数实现 ------------------------------------------------------------------*/

/**
 * @brief 延时函数初始化
 * @param sys_clk 系统时钟频率（单位：MHz）
 * @note 当前版本未使用此参数，保留用于后续扩展
 */
void delay_init(uint8_t sys_clk)
{
    (void)sys_clk;  // 避免未使用参数警告
}

/**
 * @brief 简单的循环延时函数
 * @param count 延时循环次数
 * @note 这是一个纯软件延时，延时时间取决于CPU频率
 *       每次循环大约4个时钟周期 (1个减法+1个比较+1个跳转+1个__NOP)
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
 * @note 基于SysTick定时器的精确延时，通过调用delay_us实现
 * @warning 此函数依赖于SysTick定时器正常工作
 */
void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        delay_us(1000);  
    }
}

/**
 * @brief 微秒级延时函数
 * @param us 延时时间（单位：微秒）
 * @note 基于SysTick定时器的精确延时实现
 *       1. 计算需要的中断次数 (us/10，向上取整)
 *       2. 设置TimingDelay计数器
 *       3. 等待SysTick_Handler递减计数器至0
 * @warning 此函数依赖于SysTick定时器初始化且中断使能
 */
void delay_us(uint32_t us)
{
    TimingDelay = us / 10 ;           // 计算10us的个数
    if (us % 10)                      // 不足10us的部分也按10us计算
    {
        TimingDelay++;
    }
    
    while(TimingDelay != 0);          // 等待延时完成
}

/**
 * @brief SysTick定时器初始化
 * @note 配置SysTick定时器以100kHz频率产生中断（每10us一次）
 *       中断频率 = SystemCoreClock / (100000 - 1)
 * @warning 此函数会配置系统级别的SysTick定时器，请确保没有冲突
 */
void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock/100000))
    {
        while(1);  // 配置失败则死循环
    }
}

/**
 * @brief 获取系统时钟计数
 * @return uint32_t 返回自SysTick启动以来的毫秒数
 * @note 此计数器在SysTick_Handler中每1ms递增一次
 */
uint32_t get_systick(void)
{
    return Systick_count;
}
