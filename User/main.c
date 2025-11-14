#include "led.h"
#include "key.h"
#include "beep.h"
// #include "debug.h"  // 移植到uart_dma.h中
#include "uart_dma.h"    // 使用DMA版本的调试函数
#include "dht11.h"
#include <stdio.h>

// 函数声明
void ls_interruptible(void);
void ysysled(void);

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
		delay_ms(130);
	}
}
void ysysled()
{
	uint32_t last_LED0 = 0;
	uint32_t last_LED1 = 0;
	while (1)
	{
		if (KEY_Get() != 0)
		{
			return;
		}
		if ((get_systick() - last_LED0) >= 500)
		{
			LED0 = !LED0;
			last_LED0 = get_systick();
		}

		if ((get_systick() - last_LED1) >= 800)
		{
			LED1 = !LED1;
			last_LED1 = get_systick();
		}
	}
}

/**
 * @brief 主函数
 */
int main(void)
{
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 中断分组配置一次就行
	debug_init();
	uint8_t key;					 // 保存键值
	SysTick_Init();	 // 初始化延时函数
	LED_Init();			 // 初始化LED端
	KEY_Init();			 // 初始化与按键连接的硬件接口
	DHT11_Init();      // 初始化DHT11（在延时函数初始化后）
	LED_Set_All(0);	 // 点亮所有LED（低电平点亮）
	// 实际效果：LED0、LED1、LED2亮，LED3灭（红色LED3未点亮）
	
	// 发送初始化完成消息
	printf("System Ready - Send commands to control LEDs\r\n");
	Usart1_Send_String("Press keys to control remote LEDs\r\n");
	DHT11_Data_TypeDef DHT11_Data;

	while (1)
	{
		key = KEY_Get(); // 从中断获取键值
		// 处理串口命令
		Process_Usart_Command();
		
		if (key)
		{
			uint8_t result;
			switch (key)
			{
			case KEY0_PRES: // 读取温湿度数据
				
				printf("k0 pers\n");
				result = Read_DHT11(&DHT11_Data);
				if (result == 0) {
					printf("Temperature: %d.%dC, Humidity: %d.%d\r\n",  DHT11_Data.temp_int,  DHT11_Data.temp_deci , DHT11_Data.humi_int,DHT11_Data.humi_deci );
				} else {
					printf("Failed to read DHT11 sensor data\r\n");
				}
				break;
			case KEY1_PRES: // 控制LED1翻转
				// Usart1_Send_String("1c\r\n");
				printf("123456789qwertyuiopasdfgjj\n");
				break;
			case KEY2_PRES: // 控制LED2翻转
				Usart1_Send_String("2c\r\n");
				break;
			case KEY3_PRES: // 控制LED3翻转
				Usart1_Send_String("3c\r\n");
				break;
			}
		}
		else
			delay_ms(10);
	}
}
