#include <stm32f4xx.h>
#include <FreeRTOS.h>
#include <task.h>
#include <hooks.h>

#include "key.h"
#include "oled.h"
#include "oled_print.h"
#include "uart_dma.h"
#include "soft_i2c.h"
#include "logo.h"
#include "ui.h"
#include "ui/alarm_all.h"
#include "rtc_date.h" // ????RTC????
#include "MPU6050.h"
#include "MPU6050/eMPL/inv_mpu_dmp_motion_driver.h"
#include "simple_pedometer.h"
#include <stdlib.h> // ????abs????????
#include "iwdg.h"
#include "beep.h"

static TaskHandle_t app_task1_handle = NULL;
static TaskHandle_t app_main_task_handle = NULL;
static void app_task1(void *pvParameters);
static void app_main_task(void *pvParameters);

static TaskHandle_t app_task2_handle = NULL;
static void app_task2(void *pvParameters);
// 全局变量声明
u8 key;
u8 cho = 0;
unsigned long loop_counter = 0;
unsigned long last_count = 0;

// ????????
#define options_NUM 7

/**
 * @brief ???????????
 * @param weekday ???????1-7??1=???????
 * @return ?????????????
 */
static const char *get_weekday_name(u8 weekday)
{
	static const char *weekday_names[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
	if (weekday >= 1 && weekday <= 7)
	{
		return weekday_names[weekday - 1];
	}
	return "---";
}

// ????????
const unsigned char *options[] =
		{
				gImage_stopwatch,
				gImage_setting,
				gImage_TandH,
				gImage_flashlight,
				gImage_bell,
				gImage_step,
				gImage_test

};

u8 enter_select(u8 selected)
{
	switch (selected)
	{
	case 0:
		stopwatch();
		break;
	case 1:
		setting();
		break;
	case 2:
		TandH();

		break;
	case 3:
		flashlight();
		break;
	case 4:
		alarm_menu();
		break;

	case 5:
		step(); // ????????
		break;
	case 6:
		testlist();
		break;
	default:
		break;
	}
	return selected;
}

void menu_Refresh(u8 selected)
{
	u8 left;
	if (selected == 0)
	{
		left = options_NUM - 1;
	}
	else
	{
		left = selected - 1;
	}

	u8 right = ((selected + 1) % options_NUM);

	OLED_ShowPicture(0, 16, 32, 32, options[left], 1);
	OLED_ShowPicture(48, 16, 32, 32, options[selected], 0);
	OLED_ShowPicture(96, 16, 32, 32, options[right], 1);
	OLED_Refresh();
}
// ?????????????????????????��??��???
u8 menu(u8 cho)
{

	delay_ms(10);
	u8 flag_RE = 1;
	u8 selected = cho;
	u32 current_time = get_systick();
	u8 key;
	while (1)
	{
		delay_ms(10);
		IWDG_ReloadCounter();
		// 全局闹钟处理 - 在菜单界面也能处理闹钟
		if (Alarm_GlobalHandler())
		{
			continue; // 如果正在处理闹钟提醒，跳过菜单循环的其他部分
		}

		if (flag_RE)
		{
			OLED_Clear();
			menu_Refresh(selected);
			current_time = get_systick();
			flag_RE = 0;
		}

		if ((key = KEY_Get()) != 0)
		{
			if (get_systick() - current_time > 500)
			{
				switch (key)
				{
				case KEY0_PRES:
					if (selected == 0)
					{
						selected = options_NUM - 1; // 0?????????
					}
					else
					{
						selected--;
					}
					menu_Refresh(selected);

					break;
				case KEY1_PRES:
					selected++;
					selected = selected % options_NUM;
					menu_Refresh(selected);
					break;
				case KEY2_PRES:
					OLED_Clear();
					return selected;

				case KEY3_PRES:
					flag_RE = 1;
					selected = enter_select(selected); // ???????????????
					break;

				default:
					break;
				}
			}
		}
	}
}
  #define STACK_SIZE 200
  StaticTask_t xTaskBuffer1;
	StaticTask_t xTaskBuffer2;
	  StackType_t xStack1[ STACK_SIZE ];
		StackType_t xStack2[ STACK_SIZE ];

int main(void)
{

	LED_Init();
	LED_Set_All(1);
TaskHandle_t xHandle = NULL;
	/* 创建app_main_task任务 */
	xTaskCreate((TaskFunction_t)app_main_task,					/* 任务入口函数 */
							(const char *)"app_main_task",					/* 任务名字 */
							(uint16_t)2048,													/* 任务栈大小 - 增大栈空间 */
							(void *)NULL,														/* 任务入口函数参数 */
							(UBaseType_t)4,													/* 任务的优先级 */
							(TaskHandle_t *)&app_main_task_handle); /* 任务控制块指针 */

	/* Create the task without using any dynamic memory allocation. */
	xHandle = xTaskCreateStatic(
			app_task1,				/* Function that implements the task. */
			"app_task1",						/* Text name for the task. */
			STACK_SIZE,				/* Number of indexes in the xStack array. */
			(void *)1,				/* Parameter passed into the task. */
			tskIDLE_PRIORITY, /* Priority at which the task is created. */
			xStack1,						/* Array to use as the task's stack. */
			&xTaskBuffer1);		/* Variable to hold the task's data structure. */

			xHandle = xTaskCreateStatic(
			app_task2,				/* Function that implements the task. */
			"app_task2",						/* Text name for the task. */
			STACK_SIZE,				/* Number of indexes in the xStack array. */
			(void *)1,				/* Parameter passed into the task. */
			tskIDLE_PRIORITY, /* Priority at which the task is created. */
			xStack2,						/* Array to use as the task's stack. */
			&xTaskBuffer2);		/* Variable to hold the task's data structure. */
	/* 开启任务调度 */
	vTaskStartScheduler();
}

static void app_task1(void *pvParameters)
{
	while (1)
	{

		LED1 = !LED1;

		vTaskDelay(pdMS_TO_TICKS(2000)); // 延时10ms，避免占用过多CPU时间
	}
}
static void app_task2(void *pvParameters)
{
	while (1)
	{
		LED2 = !LED2;
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

static void app_main_task(void *pvParameters)
{
	debug_init();
	printf("\ndebug init OK:");
	printf("------------------------------------------------------>>\r\n");
	KEY_Init();
	printf("key init OK\r\n");
	OLED_Init();

	OLED_ShowPicture(32, 0, 64, 64, gImage_bg, 1);
	// 初始化RTC
	RTC_Date_Init();

	// 初始化闹钟系统
	Alarms_Init();

	OLED_Refresh(); // ????
	printf("\r\n");
	MPU_Init();

	// ??MPU6050??ID
	u8 device_id;
	MPU_Read_Byte(MPU_ADDR, MPU_DEVICE_ID_REG, &device_id);
	printf("MPU6050 Device ID: 0x%02X (Expected: 0x68)\r\n", device_id);

	if (device_id != MPU_ADDR)
	{
		printf("MPU6050 Device ID Mismatch!\r\n");
		OLED_Printf_Line(1, "MPU ID Error!");
		OLED_Refresh();
		delay_ms(2000);
	}
	else
	{
		printf("MPU6050 Device ID OK\r\n");
	}

	// ????????
	simple_pedometer_init();

	// ????????????DMP????????????????
	SPI1_Init();

	u8 key;
	u8 cho = 0;
	unsigned long last_count = 0;
	BEEP_Init();

	// ???????????
	static unsigned long loop_counter = 0;
	OLED_Clear();

	// RTC_SetTime_Manual(23, 59, 57);
	printf("\r\n");
	printf("<<----------------------------------------------system init OK!\r\n");

	IWDG_Init();
	while (1)
	{
		// 全局闹钟处理 - 在任何界面都能处理闹钟
		if (Alarm_GlobalHandler())
		{
			delay_ms(100); // 给闹钟显示留出时间
			continue;			 // 如果正在处理闹钟提醒，跳过主循环的其他部分
		}

		IWDG_ReloadCounter();
		// 备用闹钟检查 - 防止中断失效
		Alarm_Check();

		// 获取当前时间用于自动测试
		RTC_Date_Get(); // 确保获取最新的RTC时间

		// 只有真正到达00:00:00-00:00:30范围内才触发测试闹钟
		if (g_RTC_Time.RTC_Hours == 0 && g_RTC_Time.RTC_Minutes == 0 &&
				g_RTC_Time.RTC_Seconds <= 30 &&
				!alarm_alert_active)
		{
			// 确保真的到了00:00:00之后才触发（避免RTC时间同步问题）
			static uint8_t trigger_flag = 0;
			if (g_RTC_Time.RTC_Seconds == 0 || trigger_flag)
			{
				if (!trigger_flag)
				{
					printf("Auto midnight alarm test triggered at %02d:%02d:%02d\r\n",
								 g_RTC_Time.RTC_Hours, g_RTC_Time.RTC_Minutes, g_RTC_Time.RTC_Seconds);
					trigger_flag = 1;
					Alarm_ForceTrigger();
				}
			}
		}

		// ??RTC??
		RTC_Date_Get();
		OLED_Printf_Line(0, "%02d/%02d/%02d     %s",

										 g_RTC_Date.RTC_Year + 2000,
										 g_RTC_Date.RTC_Month,
										 g_RTC_Date.RTC_Date,
										 get_weekday_name(g_RTC_Date.RTC_WeekDay));
		// 显示时间 32像素

		OLED_Printf_Line_32(1, " %02d:%02d:%02d",
												g_RTC_Time.RTC_Hours,
												g_RTC_Time.RTC_Minutes,
												g_RTC_Time.RTC_Seconds);

		// ???????????
		short ax, ay, az;
		MPU_Get_Accelerometer(&ax, &ay, &az);

		// ???????
		loop_counter++;

		// ?????????????????????
		simple_pedometer_update(ax, ay, az);
		unsigned long count = g_step_count;

		// ??????
		if (count != last_count)
		{
			// printf("!!! STEP DETECTED: %ld -> %ld !!!\r\n", last_count, count);
			last_count = count;
		}

		// ??????
		static short last_ax = 0, last_ay = 0, last_az = 0;
		long accel_diff = abs(ax - last_ax) + abs(ay - last_ay) + abs(az - last_az);
		last_ax = ax;
		last_ay = ay;
		last_az = az;

		if (accel_diff > 5000) // ????????
		{
			// printf("Movement: diff=%ld\r\n", accel_diff);
		}

		// if (loop_counter % 10 == 0)
		// {
		// 	printf("Step: %ld\r\n", count);
		// }

		// printf("Current step: %ld\r\n", count);

		OLED_Printf_Line(3, "step : %lu", count); // ????
		int timeofdaybeuse = (g_RTC_Time.RTC_Hours * 60 + g_RTC_Time.RTC_Minutes);
		OLED_DrawProgressBar(0, 44, 125, 2, timeofdaybeuse, 0, 24 * 60, 0, 1);
		OLED_DrawProgressBar(125, 0, 2, 64, g_RTC_Time.RTC_Seconds, 0, 60, 0, 1); // ????
		OLED_Refresh_Dirty();
		delay_ms(150); // ???????????

		if ((key = KEY_Get()) != 0)
		{
			switch (key)
			{

			case KEY0_PRES:
				TandH();
				break;
			case KEY1_PRES:
				BEEP_Buzz(0, 10);
				break;
			case KEY3_PRES:
				printf("cd menu\r\n");
				cho = menu(cho);
				printf("out menu\r\n");
				break;

			default:
				break;
			}
		}
	}
}
