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
 * @brief 获取星期名称
 * @param weekday 星期几（1-7，1=星期一）
 * @return 星期名称字符串
 */
static const char* get_weekday_name(u8 weekday)
{
    static const char* weekday_names[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    if (weekday >= 1 && weekday <= 7) {
        return weekday_names[weekday - 1];
    }
    return "---";
}

/**
 * @brief 主函数
 */
// int main(void)
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
	OLED_Init();		 // 初始化OLED屏幕
	OLED_Clear();		 // 清除屏幕
	OLED_Printf_Line(1, "loading...");
	OLED_ShowPicture(64, 0, 64, 64, gImage_bgg, 1);
	OLED_Refresh(); // 更新显存，后才显示设置内容
	// MPU6050初始化
	MPU_Init();
	while (mpu_dmp_init())
	{
		printf("MPU6050 ERROR \r\n");
		OLED_Printf_Line(1, "MPU6050 ERROR");
		OLED_Refresh();
		delay_ms(500);
	}
	dmp_set_pedometer_step_count(0); // 置零0
	dmp_set_pedometer_walk_time(0);	 // 置时间0

	// 初始状态
	LED_Set_All(1); // 关闭所有的灯

	uint8_t LED_State[4]; // 用于存放led状态数组
	uint8_t mode = 0;
	uint8_t last_mode = 0;
	DHT11_Data_TypeDef dhtdata; // 读取的温湿度数据
	// 模式计时器
	uint32_t mode_timers[16] = {0}; // 支持16种模式
	const uint32_t MODE_INTERVAL[16] = {
			[1] = 1000, // MPU6050模式更新间隔
			[2] = 50,		// MPU6050陀螺仪模式更新间隔
			[3] = 1000, // 时间模式更新间隔
			[5] = 1000,
			[7] = 1000};
	uint32_t dht_timer = 0;
	uint32_t dht_interval = 3000;
	uint32_t KEY_Get_timer = 0;
	u8 flag_delay = 1;
	float lightV = 0; // 光敏电阻的电压
//  RTC_SetDateTime_Manual(25, 11, 20, 5, 18, 40, 0);
	OLED_Clear(); // 清除屏幕
	/**
		> <

		> -
		> <
		- <
		o o

	*/
	// 开机图像，oled输出
	
	// OLED_Refresh();
	// delay_ms(1000);
	
	// OLED_Refresh();
	// delay_ms(500);
	// OLED_ShowString(40, 10, ">  <", 24, 1);
	OLED_Printf_Line(0,"    system init ");
	OLED_Printf_Line(1,"         OK   ");
	OLED_Printf_Line(2,"    use button");
	OLED_Printf_Line(3,"    change mode!");

	OLED_Refresh(); // 更新显存，后才显示设置内容

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
			// 更新显存，后才显示设置内容
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
				OLED_Printf_Line(1, "      Mode 1 ");
				OLED_Printf_Line(2, " MPU6050 sensor data");
				unsigned long count;					// 计数
				short temp;										// 温度
				temp = MPU_Get_Temperature(); // 得到温度值
				dmp_get_pedometer_step_count(&count);
				printf("temp=%.2f, count=%u\n", temp / 100.0f, count);
				OLED_Refresh_Dirty();
			}
			break;

		case 2: // 展示图片
			if (current_time - mode_timers[2] > MODE_INTERVAL[3])
			{
				mode_timers[2] = current_time;
				OLED_ShowPicture(32, 5, 58, 58, gImage_1, 1);
				OLED_Refresh(); // 更新显存，后才显示设置内容

				OLED_Refresh_Dirty();
			}
			break;

		case 3: // 时间模式
			if (current_time - mode_timers[3] > MODE_INTERVAL[3])
			{
				mode_timers[3] = current_time;
				printf("Mode 3 - Get Time\r\n");
				OLED_Printf_Line(1, "Mode 3 - Get Time");
				RTC_Date_Get();
				OLED_Refresh_Dirty();
			}
			break;
		case 4:
			OLED_Printf_Line(1, "   Mode 4 ");
			OLED_Printf_Line(2, "MPU6050 sensor data");
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
			OLED_Refresh_Dirty();
			break;

		case 5: // OLED显示时间+温湿度模式
			if (current_time - mode_timers[5] > MODE_INTERVAL[5])
			{
				mode_timers[5] = current_time;

				// 增加延时，避免读取过于频繁

				// 读取RTC时间
				RTC_Date_Get();
				int result = 0;
				// 读取DHT11温湿度数据
				if (current_time - dht_timer > dht_interval)
				{
					dht_timer = current_time;
					result = Read_DHT11(&dhtdata);
				}
				// OLED_Clear(); // 注释掉整屏清除，改用局部刷新

				if (result == 0)
				{
					// 在OLED上显示日期、时间和温湿度
					OLED_Printf_Line(0, "%02d/%02d/%02d %s",
													 g_RTC_Date.RTC_Year + 2000,
													 g_RTC_Date.RTC_Month,
													 g_RTC_Date.RTC_Date,
													 get_weekday_name(g_RTC_Date.RTC_WeekDay));
					OLED_Printf_Line(1, "%02d:%02d:%02d",
													 g_RTC_Time.RTC_Hours,
													 g_RTC_Time.RTC_Minutes,
													 g_RTC_Time.RTC_Seconds);
					OLED_Printf_Line(2, "T:%d.%dC H:%d.%d%%",
													 dhtdata.temp_int, dhtdata.temp_deci,
													 dhtdata.humi_int, dhtdata.humi_deci);
					OLED_Printf_Line(3, "Mode5: Date&Time&TH");

					// 同时通过串口输出
					printf("Mode 5 - Date: %04d-%02d-%02d(%s) %02d:%02d:%02d, Temp: %d.%dC, Humi: %d.%d%%\r\n",
								 g_RTC_Date.RTC_Year + 2000, g_RTC_Date.RTC_Month, g_RTC_Date.RTC_Date,
								 get_weekday_name(g_RTC_Date.RTC_WeekDay),
								 g_RTC_Time.RTC_Hours, g_RTC_Time.RTC_Minutes, g_RTC_Time.RTC_Seconds,
								 dhtdata.temp_int, dhtdata.temp_deci,
								 dhtdata.humi_int, dhtdata.humi_deci);
				}
				else
				{
					OLED_Refresh();
					OLED_Clear();
					// 读取失败，显示日期时间和错误信息
					OLED_Printf_Line(0, "%02d/%02d/%02d %s",
													 g_RTC_Date.RTC_Year + 2000,
													 g_RTC_Date.RTC_Month,
													 g_RTC_Date.RTC_Date,
													 get_weekday_name(g_RTC_Date.RTC_WeekDay));
					OLED_Printf_Line(1, "%02d:%02d:%02d",
													 g_RTC_Time.RTC_Hours,
													 g_RTC_Time.RTC_Minutes,
													 g_RTC_Time.RTC_Seconds);
					OLED_Printf_Line(2, "DHT11 Error!");
					OLED_Printf_Line(3, "Code: %d", result);

					printf("Mode 5 - Date: %04d-%02d-%02d(%s) %02d:%02d:%02d, DHT11 failed: %d\r\n",
								 g_RTC_Date.RTC_Year + 2000, g_RTC_Date.RTC_Month, g_RTC_Date.RTC_Date,
								 get_weekday_name(g_RTC_Date.RTC_WeekDay),
								 g_RTC_Time.RTC_Hours, g_RTC_Time.RTC_Minutes, g_RTC_Time.RTC_Seconds, result);
				}

				// 刷新OLED显示（只刷新有变化的部分）
				OLED_Refresh_Dirty();
			}
			break;
		case 6: // 展示图片
			if (current_time - mode_timers[6] > MODE_INTERVAL[3])
			{
				mode_timers[6] = current_time;
				OLED_ShowPicture(32, 0, 64, 64, gImage_bg, 1);
				OLED_Refresh(); // 更新显存，后才显示设置内容

				OLED_Refresh_Dirty();
			}
			break;

		case 7:
			flag_delay = 0; // 确保不会delay阻塞模式切换
			if (current_time - mode_timers[7] > MODE_INTERVAL[7])
			{
				mode_timers[7] = current_time;
				
				// 强制ADC重新配置并清除缓存
				ADC_Cmd(ADC3, DISABLE);  // 关闭ADC
				delay_ms(5);
				ADC_Cmd(ADC3, ENABLE);   // 重新开启ADC
				delay_ms(10);             // 等待ADC稳定
				
				// 重新配置通道
				ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_56Cycles);
				delay_ms(5);
				
				// 启动ADC转换并等待完成，添加超时保护
				uint32_t timeout = 100000;
				ADC_SoftwareStartConv(ADC3);
				while(ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET && timeout--);
				
				if(timeout == 0) {
					printf("ADC timeout!\r\n");
					lightV = 1.5f; // 使用默认值
				} else {
					uint16_t raw_adc = ADC_GetConversionValue(ADC3);
					lightV = ((float)raw_adc / 4095.0f) * 3.3f;
					
					// 检查ADC值是否合理
					if(raw_adc > 4095 || lightV > 3.3f) {
						printf("Invalid ADC reading: Raw=%d, V=%.2f\r\n", raw_adc, lightV);
						lightV = 1.5f; // 使用默认值
					} else {
						printf("ADC3 Ch5: %.2fV (Raw: %d)\r\n", lightV, raw_adc);
					}
				}
				
				// 测试变化检测
				static float last_lightV = -1;
				float delta = 0;
				if(last_lightV >= 0) {
					delta = (lightV - last_lightV);
					if(delta < 0) delta = -delta;
				}
				printf("Delta: %.3fV\r\n", delta);
				last_lightV = lightV;
				
				// 清理显示区域 - 更精确的显示
				OLED_Printf_Line(0, "     Mode 7     ");
				OLED_Printf_Line(1, " Light Sensor  ");
				OLED_Printf_Line(2, "%.2fV D:%.3f", lightV, delta);
				
				// 根据电压显示不同状态
				if(lightV > 2.0f) {
					OLED_Printf_Line(3, " -  <");
				} else if(lightV > 1.5f) {
					OLED_Printf_Line(3, " -  -");
				} else if(lightV > 1.2f) {
					OLED_Printf_Line(3, " >  -");
				} else {
					OLED_Printf_Line(3, " >  <");
				}
				
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
