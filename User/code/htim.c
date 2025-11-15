#include "htim.h"

void TIM14_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
    
    // 2. 配置GPIOF Pin9为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // 3. 将PF9连接到TIM14（复用功能映射）
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);
    
    // 4. 配置TIM14时基单元(TIM4内部时钟源默认84M)
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;           	// 预分频器84-1
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseStructure.TIM_Period = 999;             		// 自动重装载值1000-1
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 时钟分频(死区和输入采样时才有用）
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数次数，高级定时器才有
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
    
    // 5. 配置TIM14通道1为PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;   	// PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;// 互补功能,只有高级定时器才有
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;    // 输出极性高
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;	// 定时器处于空闲状态（非工作状态）时，输出通道的电平状态,只有高级定时器才有
    TIM_OCInitStructure.TIM_Pulse = 0;     			// 比较寄存器数值
    TIM_OC1Init(TIM14, &TIM_OCInitStructure);		// 配置输出模式
	
	//在更新事件发生时才将新的比较值从预装载寄存器传输到活动寄存器。
    TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);  // 使能预装载
    
    // 6. 使能TIM14预分频器自动重装载
	// 在更新事件发生时才将新的ARR值从预装载寄存器传输到活动寄存器。
    TIM_ARRPreloadConfig(TIM14, ENABLE);
    
    // 7. 启动TIM14
    TIM_Cmd(TIM14, ENABLE);
}

// 设置PWM占空比百分比 (0-100)
void Set_PWM_Percentage1(uint8_t percentage)
{
    if(percentage > 100)
        percentage = 100;
    
    uint16_t duty_cycle = (uint16_t)((percentage * 1000) / 100);
    if(duty_cycle > 1000)
        duty_cycle = 1000;
    TIM_SetCompare1(TIM14, duty_cycle);  // 设置比较寄存器数值
}
