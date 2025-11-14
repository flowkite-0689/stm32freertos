/**
 * @file uart_dma.c
 * @brief STM32串口DMA驱动模块
 * @details 提供基于DMA的串口通信功能，包括接收、发送和命令处理
 *          支持LED控制命令，通过串口接收指令控制LED开关状态
 * @author Developer
 * @date 2024
 */

#include "uart_dma.h"
#include "led.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief DMA接收缓冲区
 * @details 存储通过DMA接收到的串口数据，大小为128字节
 */
uint8_t rx_buffer[128] = {0};

/**
 * @brief DMA发送缓冲区
 * @details 存储待发送的数据，通过DMA传输到串口，大小为128字节
 */
uint8_t tx_buffer[128] = {0};

/**
 * @brief 串口接收缓冲区
 * @details 存储接收到的命令字符串，最大长度为64字节
 */
static char usart_rx_buffer[64];

/**
 * @brief 串口接收索引
 * @details 当前接收缓冲区的写入位置索引
 */
static uint16_t usart_rx_index = 0;

/**
 * @brief 命令就绪标志
 * @details 标志位，1表示接收到完整命令，可以处理
 */
static uint8_t command_ready = 0;

/**
 * @brief 接收计数器
 * @details 用于统计接收到的字节数，主要用于测试中断功能
 */
static uint32_t rx_count = 0;

/**
 * @brief 打印数组内容
 * @details 以十六进制格式打印数组中的每个字节
 * 
 * @param arr 待打印的数组指针
 * @param len 数组长度
 */
void printf_array(uint8_t *arr, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%#x ", arr[i]);
    }
    printf("\n");
}

/**
 * @brief USART1中断服务函数
 * @details 处理USART1接收中断，当接收到空闲帧时触发，处理DMA接收的数据
 */
void USART1_IRQHandler(void)
{
    uint16_t len = 0;
    // 判断接收标志位是否置1
    if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET)
    {
        // 正确的IDLE中断清除方式：先读SR，再读DR
        volatile uint16_t temp;
        temp = USART1->SR; // 读状态寄存器
        temp = USART1->DR; // 读数据寄存器 - 这是清除IDLE标志的关键！

        // 暂停DMA，方便处理接收到的数据
        DMA_Cmd(DMA2_Stream5, DISABLE);

        // 计算接收到的数据长度
        // DMA_GetCurrDataCounter(DMA2_Stream5) 返回剩余未搬数据项
        len = sizeof(rx_buffer) / sizeof(rx_buffer[0]) - DMA_GetCurrDataCounter(DMA2_Stream5); // 接收了多少个数据项
        len = len ? len : sizeof(rx_buffer) / sizeof(rx_buffer[0]);                            // 如果len=0，证明接收的数据是刚好存满到数组里面

        // 处理接收到的数据
        for (uint16_t i = 0; i < len; i++)
        {
            uint8_t temp_char = rx_buffer[i];
            rx_count++;  // 增加接收计数

            // 如果是回车或换行，命令结束
            if (temp_char == '\r' || temp_char == '\n')
            {
                if (usart_rx_index > 0)
                {
                    usart_rx_buffer[usart_rx_index] = '\0'; // 字符串结束符
                    command_ready = 1;                      // 标记命令准备就绪
                    usart_rx_index = 0;                     // 重置索引
                }
            }
            // 普通字符，存入缓冲区
            else if (usart_rx_index < sizeof(usart_rx_buffer) - 1)
            {
                usart_rx_buffer[usart_rx_index++] = temp_char;
            }
        }

        // 重置DMA接收，为下一次接收做准备
        DMA_SetCurrDataCounter(DMA2_Stream5, sizeof(rx_buffer) / sizeof(rx_buffer[0]));
        DMA_Cmd(DMA2_Stream5, ENABLE);
    }
}

/**
 * @brief DMA2_Stream7中断服务函数
 * @details 处理DMA发送完成中断，当DMA传输完成时触发
 */
void DMA2_Stream7_IRQHandler(void)
{
    // 检查DMA传输完成中断
    if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
    {
        // 清除DMA传输完成中断标志
        DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
        
        // DMA传输完成，可以禁用DMA发送
        DMA_Cmd(DMA2_Stream7, DISABLE);
        USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
    }
}

/**
 * @brief USART1初始化函数
 * @details 配置USART1的GPIO、参数和中断，为DMA通信做准备
 */
static void usart1_init(void)
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

    // 4）配置中断控制器并使能USART接收空闲中断
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

    // 5）设置中断优先级（如果需要开启串口中断才需要这个步骤）
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn; // 中断通道(中断源)
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 6）使能USART；
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief USART1 DMA接收初始化
 * @details 配置DMA2的Stream5用于USART1数据接收，使用循环模式持续接收数据
 */
void usart1_dma_rx_init(void)
{
    DMA_InitTypeDef DMA_InitStruct;

    // 1）DMA时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    // 2）启用并配置连接到DMA的外设
    usart1_init();

    // 3）DMA参数配置
    DMA_StructInit(&DMA_InitStruct); // 初始化DMA配置结构体为默认值(可选但推荐)

    DMA_InitStruct.DMA_Channel = DMA_Channel_4;                    // 通道4
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; // 外设地址
    DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)rx_buffer;      // 内存地址
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;                      // 外设到内存
    DMA_InitStruct.DMA_BufferSize = sizeof(rx_buffer) / sizeof(rx_buffer[0]); // 传输外设的数据项个数
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;             // 外设地址不递增
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                      // 内存地址递增
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;      // 外设数据宽度8位
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;              // 内存数据宽度8位
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                              // 循环模式，如果接收数据超出数组大小范围那么从头开始覆盖写入数组
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;                        // 优先级中
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;                       // 禁用fifo（这里数据宽度一样，因此禁用）
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;            // fifo阈值
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;                  // 突发：单次传输
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;          // 突发：单次传输
    DMA_Init(DMA2_Stream5, &DMA_InitStruct);  // DMA参数配置

    // 6）使能DMA
    DMA_Cmd(DMA2_Stream5, ENABLE);

    // 7）激活外设所需的流请求（内部SRAM / FLASH存储器除外）
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}

/**
 * @brief USART1 DMA发送初始化
 * @details 配置DMA2的Stream7用于USART1数据发送，使用正常模式，配置传输完成中断
 */
void usart1_dma_tx_init(void)
{
    DMA_InitTypeDef DMA_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1）DMA时钟使能（已在接收函数中使能，这里可以省略）
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    // 2）配置DMA发送参数
    DMA_StructInit(&DMA_InitStruct);

    DMA_InitStruct.DMA_Channel = DMA_Channel_4;                    // 通道4
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; // 外设地址
    DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)tx_buffer;      // 内存地址
    DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;           // 内存到外设
    DMA_InitStruct.DMA_BufferSize = 0;                             // 传输数据项个数（发送时动态设置）
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  // 外设地址不递增
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;           // 内存地址递增
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据宽度8位
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;   // 内存数据宽度8位
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;                     // 正常模式（非循环）
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;             // 优先级中
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;            // 禁用fifo
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;  // fifo阈值
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;       // 突发：单次传输
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // 突发：单次传输
    DMA_Init(DMA2_Stream7, &DMA_InitStruct);

    // 3）配置DMA发送完成中断（可选）
    DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
    NVIC_InitStruct.NVIC_IRQChannel = DMA2_Stream7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 4）初始化时先不使能DMA，发送数据时再使能
    DMA_Cmd(DMA2_Stream7, DISABLE);
}

/**
 * @brief 使用DMA发送数据
 * @details 通过DMA将数据从内存发送到USART1
 * 
 * @param data 待发送数据的指针
 * @param len 待发送数据的长度
 */
void Usart1_Send_DMA(uint8_t *data, uint16_t len)
{
    // 等待DMA传输完成（如果上一次传输还没完成）
    while (DMA_GetCmdStatus(DMA2_Stream7) == ENABLE);
    
    // 复制数据到发送缓冲区
    for (uint16_t i = 0; i < len && i < sizeof(tx_buffer); i++)
    {
        tx_buffer[i] = data[i];
    }
    
    // 设置传输数据长度
    DMA_SetCurrDataCounter(DMA2_Stream7, len);
    
    // 使能DMA发送
    DMA_Cmd(DMA2_Stream7, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

/**
 * @brief 通过串口1发送字符串
 * @details 兼容原有函数，使用DMA发送字符串数据
 * 
 * @param string 待发送的字符串指针
 */
void Usart1_Send_String(char *string)
{
    uint16_t len = 0;
    char *p = string;
    
    // 计算字符串长度
    while (*p != '\0')
    {
        len++;
        p++;
    }
    
    // 如果字符串长度为0，直接返回
    if (len == 0)
        return;
    
    // 使用DMA发送
    Usart1_Send_DMA((uint8_t *)string, len);
}

/**
 * @brief 获取接收计数
 * @details 返回当前接收到的字节数
 * 
 * @return uint32_t 接收到的字节总数
 */
uint32_t get_usart_rx_count(void)
{
    return rx_count;
}

/**
 * @brief 检查命令是否准备就绪
 * @details 返回是否有完整的命令可以处理
 * 
 * @return uint8_t 1表示有命令准备就绪，0表示无命令
 */
uint8_t is_command_ready(void)
{
    return command_ready;
}

/**
 * @brief 从接收缓冲区获取字符串
 * @details 将接收到的命令字符串复制到用户提供的缓冲区
 * 
 * @param buffer 用户提供的缓冲区指针
 * @param size 缓冲区大小
 * @return uint8_t 1表示成功获取命令，0表示无命令或缓冲区不足
 */
uint8_t Usart1_Receive_String(char *buffer, uint16_t size)
{
    if (command_ready && buffer && size > 0)
    {
        uint16_t len = strlen(usart_rx_buffer);
        if (len < size)
        {
            strcpy(buffer, usart_rx_buffer);
            command_ready = 0; // 清除命令就绪标志
            memset(usart_rx_buffer, 0, sizeof(usart_rx_buffer));
            return 1;
        }
    }

    return 0;
}

/**
 * @brief 处理串口命令
 * @details 解析接收到的命令并执行相应的LED控制操作
 *          支持的命令包括：
 *          - led0/1/2/3 on/off: 控制单个LED
 *          - all on/off: 控制所有LED
 *          - 0c/1c/2c/3c: 切换LED状态
 *          - help: 显示帮助信息
 */
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

/**
 * @brief 兼容debug_init函数
 * @details 初始化USART1的DMA接收和发送功能
 */
void debug_init(void)
{
    usart1_dma_rx_init();
    usart1_dma_tx_init();
}

/**
 * @brief 重定向c库函数printf到串口
 * @details 使printf函数的输出通过串口DMA发送
 * 
 * @param ch 待发送的字符
 * @param f 文件指针（此处未使用）
 * @return int 发送的字符
 */
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到串口 */
    static uint8_t printf_buf[1] = {0};
    printf_buf[0] = (uint8_t)ch;
    
    /* 使用DMA发送单个字符 */
    Usart1_Send_DMA(printf_buf, 1);
    
    /* 等待发送完成 */
    while (DMA_GetCmdStatus(DMA2_Stream7) == ENABLE);

    return (ch);
}