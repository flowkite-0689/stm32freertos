#include "code/led.h"
#include "code/key.h"
#include "code/beep.h"
#include "code/uart_dma.h" // 使用DMA版本的串口调试
#include "code/dht11.h"
#include <stdio.h>
#include "code/timer_general.h"
#include "code/htim.h"
#include "code/rtc_date.h"
#include "code/adc.h"
#include "soft_i2c.h"
#include "MPU6050.h"
#include "oled.h"
#include "oled_print.h"
#include "logo.h"









/**
 * @brief 主函数
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	debug_init();

	// 系统初始化
	SysTick_Init();	 // 初始化滴时器
	LED_Init();			 // 初始化LED灯
	KEY_Init();			 // 初始化按键的硬件接口
	RTC_Date_Init(); // 初始化date
	ADC3_Init();		 // 初始化ADC3
	DHT11_Init();		 // 温湿度传感器DHT11初始化
	
	// MPU6050初始化
	MPU_Init();
	while (mpu_dmp_init())
	{
		printf("MPU6050 ERROR \r\n");
		delay_ms(500);
	}
	dmp_set_pedometer_step_count(0); // 置零0
	dmp_set_pedometer_walk_time(0);	 // 置时间0

	// 初始状态
	LED_Set_All(1);				// 关闭所有的灯
	OLED_Init();    // 初始化OLED屏幕
	uint8_t LED_State[4]; // 用于存放led状态数组
	uint8_t mode = 0;
	uint8_t last_mode = 0;
	DHT11_Data_TypeDef dhtdata;//读取的温湿度数据
	// 模式计时器
	uint32_t mode_timers[16] = {0}; // 支持16种模式
	const uint32_t MODE_INTERVAL[16] = {
			[1] = 1000, // MPU6050模式更新间隔
			[2] = 50,		// MPU6050陀螺仪模式更新间隔
			[3] = 1000, // 时间模式更新间隔
			[5] = 1000};
  uint32_t dht_timer=0;
	uint32_t dht_interval= 3000;
	uint32_t KEY_Get_timer = 0;
	u8 flag_delay = 1;



	//开机图像，oled输出
OLED_ShowPicture(0,0,64,64,gImage_bg,1);
delay_ms(1000);
 OLED_Refresh(); // 更新显存，后才显示设置内容
       
        OLED_Clear();		// 清屏
	while (1) // 主循环
	{
		Process_Usart_Command();

		// 获取LED状态并计算模式
		LED_State[0] = !LED0;
		LED_State[1] = !LED1;
		LED_State[2] = !LED2;
		LED_State[3] = !LED3;
		mode = get_Oct_LED_BIT(LED_State);
		if (!mode)
		{
			flag_delay = 1;
		}

		// 模式切换检测
		if (mode != last_mode)
		{
			printf("Mode changed from %d to %d\r\n", last_mode, mode);
			last_mode = mode;
			OLED_Refresh(); 
		   OLED_Clear();
		}

		uint32_t current_time = get_systick();

		// 模式处理
		switch (mode)
		{
		case 1: // MPU6050模式
			if (current_time - mode_timers[1] > MODE_INTERVAL[1])
			{
				mode_timers[1] = current_time;
				printf("Mode 1 - MPU6050 sensor data\r\n");

				unsigned long count;					// 计数
				short temp;										// 温度
				temp = MPU_Get_Temperature(); // 得到温度值
				dmp_get_pedometer_step_count(&count);
				printf("temp=%.2f, count=%u\n", temp / 100.0f, count);
			}
			break;

		case 2: // 展示图片
			if(current_time - mode_timers[2] > MODE_INTERVAL[3])
			{
				 mode_timers[2] =current_time;
				OLED_ShowPicture(0,0,58,58,gImage_1,1);
				OLED_Refresh(); // 更新显存，后才显示设置内容
        
       	OLED_Refresh_Dirty();
			}	
			break;

		case 3: // 时间模式
			if (current_time - mode_timers[3] > MODE_INTERVAL[3])
			{
				mode_timers[3] = current_time;
				printf("Mode 3 - Get Time\r\n");
				RTC_Date_Get();
			}
			break;
		case 4:
			flag_delay = 0;
			uint8_t res;
			float pitch, roll, yaw;		 // 欧拉角
			short aacx, aacy, aacz;		 // 加速度传感器原始数据
			short gyrox, gyroy, gyroz; // 陀螺仪原始数据
			res = mpu_dmp_get_data(&pitch, &roll, &yaw);
			if (res == 0)
			{
				MPU_Get_Accelerometer(&aacx, &aacy, &aacz);																																		// 得到加速度传感器数据
				MPU_Get_Gyroscope(&gyrox, &gyroy, &gyroz);																																		// 得到陀螺仪数据
				MPU_ReportImu(aacx, aacy, aacz, gyrox, gyroy, gyroz, (int)(roll * 100), (int)(pitch * 100), (int)(yaw * 10)); // 数据到上位机
			}
			break;

			case 5: // OLED显示时间+温湿度模式
			if (current_time - mode_timers[5] > MODE_INTERVAL[5])
			{
				mode_timers[5] = current_time;
				
				// 增加延时，避免读取过于频繁
				
				
				// 读取RTC时间
				RTC_Date_Get();
				int result=0;
				// 读取DHT11温湿度数据
				if(current_time - dht_timer >dht_interval )
				{
					dht_timer = current_time;
				    result = Read_DHT11(&dhtdata);
				}
				// OLED_Clear(); // 注释掉整屏清除，改用局部刷新
				
				if (result == 0)
				{
					// 在OLED上显示日期、时间和温湿度
					OLED_Printf_Line(0, "%02d/%02d/%02d", 
						
					                 g_RTC_Date.RTC_Year + 2000,
													  g_RTC_Date.RTC_Month,
					                 g_RTC_Date.RTC_Date
					                );
					OLED_Printf_Line(1, "%02d:%02d:%02d", 
					                 g_RTC_Time.RTC_Hours,
					                 g_RTC_Time.RTC_Minutes, 
					                 g_RTC_Time.RTC_Seconds);
					OLED_Printf_Line(2, "T:%d.%dC H:%d.%d%%", 
					                 dhtdata.temp_int, dhtdata.temp_deci,
					                 dhtdata.humi_int, dhtdata.humi_deci);
					OLED_Printf_Line(3, "Mode5: Date&Time&TH");
					
					// 同时通过串口输出
					printf("Mode 5 - Date: %04d-%02d-%02d %02d:%02d:%02d, Temp: %d.%dC, Humi: %d.%d%%\r\n", 
					       g_RTC_Date.RTC_Year + 2000, g_RTC_Date.RTC_Month, g_RTC_Date.RTC_Date,
					       g_RTC_Time.RTC_Hours, g_RTC_Time.RTC_Minutes, g_RTC_Time.RTC_Seconds,
					       dhtdata.temp_int, dhtdata.temp_deci, 
					       dhtdata.humi_int, dhtdata.humi_deci);
				}
				else
				{
						OLED_Refresh();
						OLED_Clear();
					// 读取失败，显示日期时间和错误信息
					OLED_Printf_Line(0, "%02d/%02d/%02d", 
						
					                 g_RTC_Date.RTC_Year + 2000,
													  g_RTC_Date.RTC_Month,
					                 g_RTC_Date.RTC_Date
					                );
					OLED_Printf_Line(1, "%02d:%02d:%02d", 
					                 g_RTC_Time.RTC_Hours,
					                 g_RTC_Time.RTC_Minutes, 
					                 g_RTC_Time.RTC_Seconds);
					OLED_Printf_Line(2, "DHT11 Error!");
					OLED_Printf_Line(3, "Code: %d", result);
					
					printf("Mode 5 - Date: %04d-%02d-%02d %02d:%02d:%02d, DHT11 failed: %d\r\n", 
					       g_RTC_Date.RTC_Year + 2000, g_RTC_Date.RTC_Month, g_RTC_Date.RTC_Date,
					       g_RTC_Time.RTC_Hours, g_RTC_Time.RTC_Minutes, g_RTC_Time.RTC_Seconds, result);
				}
				
				// 刷新OLED显示（只刷新有变化的部分）
				OLED_Refresh_Dirty();
			}
			break;


		default:
			// 无效模式或模式0，不做特殊处理
			break;
		}

		if (current_time - KEY_Get_timer > 15)
		{
			KEY_Get_timer = current_time;
			// 按键处理
			uint8_t key = KEY_Get();
			if (key)
			{
				switch (key)
				{
				case KEY0_PRES:
					LED0 = !LED0;
					break;
				case KEY1_PRES:
					LED1 = !LED1;
					break;
				case KEY2_PRES:
					LED2 = !LED2;
					break;
				case KEY3_PRES:
					LED3 = !LED3;
					break;
				default:
					break;
				}
			}
		}

		if (flag_delay)
		{
			delay_ms(15);
		}
	}
	
}
