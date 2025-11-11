#include "led.h"
#include "key.h"
#include "beep.h"

/**
 * @brief LED流水灯效果函数
 */
void ls(void)
{
	uint32_t led_state[4] = {0, 0, 0, 0};
	uint32_t flag = 0;
	
	for (uint32_t i = 0; i < 4;)
	{
		for (uint32_t j = 0; j < 4; j++)
		{
			led_state[j] = 1;
			if (j == i)
			{
				led_state[j] = 0;
			}
		}

		if (flag)
		{
			LED0 = led_state[0];
			LED1 = led_state[1];
			LED2 = led_state[2];
			LED3 = led_state[3];
		}
		else
		{
			LED3 = led_state[0];
			LED2 = led_state[1];
			LED1 = led_state[2];
			LED0 = led_state[3];
		}

		i++;
		if (i == 4)
		{
			flag = !flag;
		}

		i = i % 4;
		delay_ms(120);
	}
}

/**
 * @brief 可中断的LED流水灯效果函数
 */
void ls_interruptible(void)
{
	uint32_t led_state[4] = {0, 0, 0, 0};
	uint32_t flag = 0;
	
	for (uint32_t i = 0; i < 4;)
	{
		// 检查是否有按键中断，如果有则退出
		if (KEY_Get() != 0)
		{
			return;
		}
		
		for (uint32_t j = 0; j < 4; j++)
		{
			led_state[j] = 1;
			if (j == i)
			{
				led_state[j] = 0;
			}
		}

		if (flag)
		{
			LED0 = led_state[0];
			LED1 = led_state[1];
			LED2 = led_state[2];
			LED3 = led_state[3];
		}
		else
		{
			LED3 = led_state[0];
			LED2 = led_state[1];
			LED1 = led_state[2];
			LED0 = led_state[3];
		}

		i++;
		if (i == 4)
		{
			flag = !flag;
		}

		i = i % 4;
		delay_ms(120);
	}
}

/**
 * @brief 主函数
 */
int main(void)
{
	u8 key;					 // 保存键值
	delay_init(168); // 初始化延时函数
	LED_Init();			 // 初始化LED端口
	BEEP_Init();		 // 初始化蜂鸣器端口
	KEY_Init();			 // 初始化与按键连接的硬件接口
	KEY_EXTI_Init(); // 初始化按键外部中断
	LED_Set_All(0);	// 点亮所有LED（低电平点亮）
	LED0 = 0;	// 熄灭LED0
	LED1 = 1;	// 熄灭LED1
	LED2 = 0;	// 熄灭LED2
	LED3 = 1;	// 熄灭LED3
	// 实际效果：LED0、LED1、LED2亮，LED3灭（红色LED3未点亮）

	while (1)
	{
		key = KEY_Get(); // 从中断获取键值
		if (key)
		{
			switch (key)
			{
			case KEY0_PRES: // 启动流水灯效果
				ls_interruptible();
				break;
			case KEY1_PRES: // 控制LED0翻转
				LED0 = !LED0;
				break;
			case KEY2_PRES: // 控制LED1翻转
				LED1 = !LED1;
				break;
			case KEY3_PRES: // 控制LED2翻转
				LED2 = !LED2;
				break;
			}
		}
		else
			delay_ms(10);
	}
}
