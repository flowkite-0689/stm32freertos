// 移植到uart_dma.c中，以下函数被注释掉以避免冲突
/*
#include "debug.h"
#include <stdio.h>
#include <string.h>
#include "led.h"

static char usart_rx_buffer[USART_RX_BUFFER_SIZE];
static uint16_t usart_rx_index = 0;
static uint8_t command_ready = 0;
static uint32_t rx_count = 0; // 计数器，用于测试中断
// USART接收中断服务函数，支持字符串接收
void USART1_IRQHandler(void)
{
    uint8_t temp = 0;
    // 判断接收标志位是否置1
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); // 清除接收标志位

        temp = USART_ReceiveData(USART1); // 读取数据
        rx_count++;                       // 增加接收计数

        // 如果是回车或换行，命令结束
        if (temp == '\r' || temp == '\n')
        {
            if (usart_rx_index > 0)
            {
                usart_rx_buffer[usart_rx_index] = '\0'; // 字符串结束符
                command_ready = 1;                      // 标记命令准备就绪
                usart_rx_index = 0;                     // 重置索引
            }
        }
        // 普通字符，存入缓冲区
        else if (usart_rx_index < USART_RX_BUFFER_SIZE - 1)
        {
            usart_rx_buffer[usart_rx_index++] = temp;
        }
    }
}

// 获取接收计数
uint32_t get_usart_rx_count(void)
{
    return rx_count;
}

// 检查命令是否准备就绪
uint8_t is_command_ready(void)
{
    return command_ready;
}

// 检查是否有命令准备好
uint8_t Usart1_Receive_String(char *buffer, uint16_t size)
{
    if (command_ready && buffer && size > 0)
    {
        uint16_t len = strlen(usart_rx_buffer);
        if (len < size)
        {
            strcpy(buffer, usart_rx_buffer);
            command_ready = 0; // 清除命令就绪标志
            memset(usart_rx_buffer, 0, USART_RX_BUFFER_SIZE);
            return 1;
        }
    }

    return 0;
}

// 处理串口命令
void Process_Usart_Command(void)
{
    char cmd[64];
    if (Usart1_Receive_String(cmd, sizeof(cmd)))
    {
        // 转换为小写便于比较
        for (int i = 0; cmd[i]; i++)
        {
            if (cmd[i] >= 'A' && cmd[i] <= 'Z')
                cmd[i] = cmd[i] + 32;
        }

        // 解析命令
        if (strcmp(cmd, "led0 on") == 0)
        {
            LED0 = 0;
            Usart1_Send_String("LED0 ON\r\n");
        }
        else if (strcmp(cmd, "led0 off") == 0)
        {
            LED0 = 1;
            Usart1_Send_String("LED0 OFF\r\n");
        }
        else if (strcmp(cmd, "led1 on") == 0)
        {
            LED1 = 0;
            Usart1_Send_String("LED1 ON\r\n");
        }
        else if (strcmp(cmd, "led1 off") == 0)
        {
            LED1 = 1;
            Usart1_Send_String("LED1 OFF\r\n");
        }
        else if (strcmp(cmd, "led2 on") == 0)
        {
            LED2 = 0;
            Usart1_Send_String("LED2 ON\r\n");
        }
        else if (strcmp(cmd, "led2 off") == 0)
        {
            LED2 = 1;
            Usart1_Send_String("LED2 OFF\r\n");
        }
        else if (strcmp(cmd, "led3 on") == 0)
        {
            LED3 = 0;
            Usart1_Send_String("LED3 ON\r\n");
        }
        else if (strcmp(cmd, "led3 off") == 0)
        {
            LED3 = 1;
            Usart1_Send_String("LED3 OFF\r\n");
        }
        else if (strcmp(cmd, "all on") == 0)
        {
            LED0 = LED1 = LED2 = LED3 = 0;
            Usart1_Send_String("ALL LEDS ON\r\n");
        }
        else if (strcmp(cmd, "all off") == 0)
        {
            LED0 = LED1 = LED2 = LED3 = 1;
            Usart1_Send_String("ALL LEDS OFF\r\n");
        }
        else if (strcmp(cmd, "help") == 0)
        {
            Usart1_Send_String("Commands:\r\n");
            Usart1_Send_String("led0/1/2/3 on/off - Control individual LED\r\n");
            Usart1_Send_String("all on/off - Control all LEDs\r\n");
        }
        else if (strcmp(cmd, "0c") == 0)
        {
            LED0 = !LED0;
            Usart1_Send_String("LED0 change\r\n");
        }
        else if (strcmp(cmd, "1c") == 0)
        {
            LED1 = !LED1;
            Usart1_Send_String("LED1 change\r\n");
        }
        else if (strcmp(cmd, "2c") == 0)
        {
            LED2 = !LED2;
            Usart1_Send_String("LED2 change\r\n");
        }
        else if (strcmp(cmd, "3c") == 0)
        {
            LED3 = !LED3;
            Usart1_Send_String("LED3 change\r\n");
        }
        else
        {
            // Usart1_Send_String("Unknown command. Type 'help'\r\n");
        }
    }
}

void debug_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1）使能RX和TX引脚GPIO时钟和USART时钟；
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // for USART1 and USART6

    // 2）初始化GPIO，并将GPIO复用到USART上
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;     // 复用模式
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; // 高速
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   // 输出模式时有用
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    // 3）配置USART参数；
    USART_InitStruct.USART_BaudRate = 115200;                                    // 波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;                     // 8位有效数据位
    USART_InitStruct.USART_StopBits = USART_StopBits_1;                          // 1位停止位
    USART_InitStruct.USART_Parity = USART_Parity_No;                             // 无校验
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 接收和发送数据模式
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件控制流
    USART_Init(USART1, &USART_InitStruct);

    // 4）配置中断控制器并使能USART接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 5）设置中断优先级（如果需要开启串口中断才需要这个步骤）
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn; // 中断通道(中断源)
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 6）使能USART；
    USART_Cmd(USART1, ENABLE);
}

// 通过串口1，单片机发送字符串
void Usart1_Send_String(char *string)
{

    while (*string != '\0')
    {
        USART_SendData(USART1, *string++);
        // 等待发送数据寄存器空
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
    }
    // 等待发送完成
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        ;
}

// 重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    // 发送一个字节数据到串口
    USART_SendData(USART1, (uint8_t)ch);

    // 等待发送完毕
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}
*/
