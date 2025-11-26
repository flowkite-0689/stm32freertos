#include <stm32f4xx.h>
#include <FreeRTOS.h>
#include <task.h>
#include <hooks.h>

static TaskHandle_t app_task1_handle = NULL;

/* 任务1 */
static void app_task1(void *pvParameters);

int main(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;        // 选择要配置的引脚：PF9引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;    // 设置引脚工作模式：输出模式
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; // 设置输出速度：高速（通常为50MHz或100MHz）
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   // 设置输出类型：推挽输出（能输出高电平和低电平），输出模式时有用
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;     // 设置上下拉电阻：上拉（引脚默认电平为高），输入模式时有用
    GPIO_Init(GPIOF, &GPIO_InitStruct);
    GPIO_ResetBits(GPIOF, GPIO_Pin_9);

	/* 创建app_task1任务 */
	xTaskCreate((TaskFunction_t)app_task1,			/* 任务入口函数 */
				(const char *)"app_task1",			/* 任务名字 */
				(uint16_t)512,						/* 任务栈大小 */
				(void *)NULL,						/* 任务入口函数参数 */
				(UBaseType_t)4,						/* 任务的优先级 */
				(TaskHandle_t *)&app_task1_handle); /* 任务控制块指针 */

	/* 开启任务调度 */
	vTaskStartScheduler();
}

static void app_task1(void *pvParameters)
{
	for (;;)
	{

		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
		vTaskDelay(1000);
	}
}
// 后面的钩子函数没列举，自行补全
