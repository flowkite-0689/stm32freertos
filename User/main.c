#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "debug.h"
#include "led.h"
#include "sys.h"
#include "event_groups.h"
#include "timers.h"
#include "hooks.h"
#include <string.h>
#include "oled_print.h"

QueueHandle_t xDataQueue;
QueueHandle_t xSendQueue;
static TaskHandle_t app_task_handle = NULL;
static TaskHandle_t LED_handle = NULL;

/* 任务1 */
static void app_task(void *pvParameters);
static void LED_task(void *pvParameters);

static TimerHandle_t xTimers[2] = {0}; /* 定时器句柄 */

void vTimerCallback(TimerHandle_t pxTimer); // 定时器回调函数





/*   === 收发uart的数据的任务===*/
static void data_task(void *pvParameters);
static void send_task(void *pvParameters);
TaskHandle_t Handle_data;
TaskHandle_t Handle_send;

/*  ===*/



int main(void)
{
    BaseType_t xReturn;
    // NVIC 分组4
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 优先级分组,freeRTOS中优先级分组不能在任务创建后修改
    debug_init();
    LED_Init();
    LED_Set_All(1); // 全部熄灭
    OLED_Init();

    xDataQueue = xQueueCreate(10, sizeof(uint8_t));
    xSendQueue = xQueueCreate(20, sizeof(uint8_t));  // 创建发送队列
    printf("main:%p", xDataQueue);
    xTaskCreate(data_task,
                "data_task",
                512,
                (void *)&xDataQueue,
                1,
                &Handle_data);
    xTaskCreate(send_task,
                "send_task",
                512,
                (void *)&xSendQueue,
                2,  // 优先级比接收任务稍高，确保及时发送
                &Handle_send);

        /* 创建app_task任务 */
        xReturn = xTaskCreate((TaskFunction_t)app_task,          /* 任务入口函数 */
                              (const char *)"app_task",          /* 任务名字 */
                              (uint16_t)512,                     /* 任务栈大小 */
                              (void *)NULL,                      /* 任务入口函数参数 */
                              (UBaseType_t)4,                    /* 任务的优先级 */
                              (TaskHandle_t *)&app_task_handle); /* 任务控制块指针 */
    if (xReturn != pdPASS)
    {
        printf("create app_task failed!\r\n");
        return 0;
    }
    /* 开启任务调度 */
    vTaskStartScheduler();
}

static void app_task(void *pvParameters)
{
    taskENTER_CRITICAL(); // 进入临界区
    /***创建定时器***/
    xTimers[0] = xTimerCreate("Timer1",            // 软件定时器名字，文本形式，纯粹是为了调试
                              pdMS_TO_TICKS(1000), // 软件定时器的周期，单位为系统节拍周期（即tick）
                              pdTRUE,              // pdTRUE:周期模式，否则运行一次
                              (void *)0,           // 软件定时器ID，数字形式。1,为每个计时器分配一个索引的唯一ID .该ID 典型的用法是当一个回调函数分配给一个或者多个软件定时器时，在回调函数里面根据ID 号来处理不同的软件定时器。
                              vTimerCallback);     // 软件定时器的回调函数

    xTimers[1] = xTimerCreate("Timer2",            // 软件定时器名字，文本形式，纯粹是为了调试
                              pdMS_TO_TICKS(1000), // 软件定时器的周期，单位为系统节拍周期（即tick）
                              pdFALSE,             // pdTRUE:周期模式，否则运行一次
                              (void *)1,           // 软件定时器ID，数字形式。该ID 典型的用法是当一个回调函数分配给一个或者多个软件定时器时，在回调函数里面根据ID 号来处理不同的软件定时器。
                              vTimerCallback);     // 软件定时器的回调函数
    if (xTimerStart(xTimers[0], 0) != pdPASS)      // 开始定时器命令,立即启动定时器
    {
        // The timer could not be set into the Active state.
    }
    if (xTimerStart(xTimers[1], 0) != pdPASS) // 开始定时器命令,立即启动定时器
    {
        // The timer could not be set into the Active state.
    }
    xTaskCreate((TaskFunction_t)LED_task,     /* 任务入口函数 */
                (const char *)"LED_task",     /* 任务名字 */
                (uint16_t)512,                /* 任务栈大小 */
                (void *)NULL,                 /* 任务入口函数参数 */
                (UBaseType_t)4,               /* 任务的优先级 */
                (TaskHandle_t *)&LED_handle); /* 任务控制块指针 */
    vTaskDelete(NULL);                        // 删除自身
    taskEXIT_CRITICAL();
}

static void LED_task(void *pvParameters)
{
    while (1)
    {
        GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
        vTaskDelay(1000);
    }
}

/**
 * @brief 定时器回调函数
 *
 * @param pxTimer 回调时定时器的句柄
 */
void vTimerCallback(TimerHandle_t pxTimer)
{
    static portCHAR lExpireCounters = 0; // 记录定时器回调次数
    int32_t lArrayIndex = 0;

    // Which timer expired?
    // 通过pvTimerGetTimerID()函数获取定时器ID
    lArrayIndex = (int32_t)pvTimerGetTimerID(pxTimer);

    switch (lArrayIndex)
    {
    case 0:
        lExpireCounters++;
        printf("xTimers1 callback\r\n");
        if (lExpireCounters == 10)
        {
            printf("xTimers1 stop\r\n");
            xTimerStop(pxTimer, 0); // 回调10次后，关闭定时器
            vTaskSuspend(LED_handle);
        }
        break;
    case 1:
        printf("xTimers2 callback\r\n");
        break;

    default:
        break;
    }
}
static void data_task(void *pvParameters)
{
    uint8_t buffer[50] = {0};

    while (1)
    {
        uint8_t ch;
        uint8_t index = 0;

        while (1)
        {
            xQueueReceive(xDataQueue, &ch, portMAX_DELAY);

            printf("%c", ch);
            if (index < sizeof(buffer) - 1)
            {
                buffer[index++] = ch;
                buffer[index] = '\0';//保证无论何时都是一个完整的字符串
            }
            if (ch == '\n')
            {
                break;
            }
        }

        if (strstr((char *)buffer, "hello"))
        {
            LED1 = !LED1;
            OLED_Printf_Line(1," LED1 = %s;",LED1?"off":"on");
        }
        if (strstr((char *)buffer, "world"))
        {
            LED2 = !LED2;
            OLED_Printf_Line(2," LED2 = %s",LED2?"off":"on");
        }
        OLED_Refresh_Dirty();

        delay_ms(10);
    }
}

static void send_task(void *pvParameters)
{
    uint8_t ch;
    
    while (1)
    {
        // 从发送队列中获取数据
        if (xQueueReceive(xSendQueue, &ch, portMAX_DELAY) == pdPASS)
        {
            // 实际发送数据
            USART_SendData(USART1, ch);
            // 等待发送数据寄存器为空
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        }
    }
}
