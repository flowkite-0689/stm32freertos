#include "iwdg.h"

// 独立看门狗配置
void IWDG_Init(void)
{
	// 使能独立看门狗时钟（LSI）
	RCC_LSICmd(ENABLE);

	// 等待LSI就绪
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	// 使能对IWDG_PR和IWDG_RLR寄存器的写访问
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	// IWDG计数器时钟：LSI(32kHz)/32 = 1kHz
	IWDG_SetPrescaler(IWDG_Prescaler_32);

	// 设置重装载值
	// 超时时间 = (重装载值 + 1) / 计数器时钟频率
	// 3000ms = (2999 + 1) / 1kHz
	IWDG_SetReload(2999); // 2秒超时

	// 重载IWDG计数器
	IWDG_ReloadCounter();

	// 启动看门狗
	IWDG_Enable();
}
