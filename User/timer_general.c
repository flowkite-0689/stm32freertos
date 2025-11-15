#include "timer_general.h"
#include "delay.h"  // 需要包含延时函数头文件
#include <stdlib.h>  // 需要包含malloc和free

void TIM13_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
    
    // 2. 配置GPIOF Pin8为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // 3. 将PF8连接到TIM13（复用功能映射）
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource8, GPIO_AF_TIM13);
    
    // 4. 配置TIM13时基单元 - 修复：使用最大Period值
    TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1;           	// 预分频器25-1，84MHz→1MHz
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseStructure.TIM_Period = 65535;             		// 修复：使用最大值，让Set_PWM_Frequency动态控制
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 时钟分频
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数次数，高级定时器才有
    TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);
    
    // 5. 配置TIM13通道1为PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;   	// PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;// 互补功能,只有高级定时器才有
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;    // 输出极性高
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;	// 定时器处于空闲状态时，输出通道的电平状态
    TIM_OCInitStructure.TIM_Pulse = 0;     			// 比较寄存器数值
    TIM_OC1Init(TIM13, &TIM_OCInitStructure);		// 配置输出模式
	
	//在更新事件发生时才将新的比较值从预装载寄存器传输到活动寄存器。
    TIM_OC1PreloadConfig(TIM13, TIM_OCPreload_Enable);  // 使能预装载
    
    // 6. 使能TIM13预分频器自动重装载
	// 在更新事件发生时才将新的ARR值从预装载寄存器传输到活动寄存器。
    TIM_ARRPreloadConfig(TIM13, ENABLE);
    
    // 7. 启动TIM13
    TIM_Cmd(TIM13, ENABLE);
}

// 设置PWM占空比百分比 (0-100)
void Set_PWM_Percentage(uint8_t percentage)
{
    if(percentage > 100)
        percentage = 100;
    
    uint16_t duty_cycle = (uint16_t)((percentage * 1000) / 100);
    if(duty_cycle > 1000)
        duty_cycle = 1000;
    TIM_SetCompare1(TIM13, duty_cycle);  // 设置比较寄存器数值
}

// 设置PWM频率以产生不同音调
void Set_PWM_Frequency(uint32_t frequency)
{
    if(frequency == 0) {
        TIM_Cmd(TIM13, DISABLE);  // 停止PWM输出
        return;
    }
    
    // 修复：正确计算period，避免整数除法误差
    // TIM13时钟为1MHz (84MHz/84)
    uint32_t period = 1000000 / frequency;  // 修复：先除后减1
    
    // 确保period在合理范围内
    if(period < 2) period = 2;  // 最小period
    if(period > 65535) period = 65535;  // 最大period
    
    // 修复：正确的ARR和CCR设置
    TIM_SetAutoreload(TIM13, period - 1);  // ARR = period - 1
    TIM_SetCompare1(TIM13, (period - 1) / 2);  // 50%占空比
    
    TIM_Cmd(TIM13, ENABLE);  // 启动PWM输出
}

