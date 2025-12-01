// /**
//  * @file uart_dma.c
//  * @brief STM32串口DMA驱动模块
//  * @details 提供基于DMA的串口通信功能，包括接收、发送和命令处理
//  *          支持LED控制命令，通过串口接收指令控制LED开关状态
//  * @author Developer
//  * @date 2024
//  */

// #include "uart_dma.h"
// #include "led.h"
// #include <string.h>
// #include <stdio.h>
// #include "FreeRTOS.h"
// #include "task.h"

// // ====================== 宏定义 ======================
// #ifndef UART_TX_BUF_SIZE
// #define UART_TX_BUF_SIZE 256  // 建议 ≥ 128，避免 printf 大量输出溢出
// #endif

// // ====================== 全局变量 ======================
// /**
//  * @brief DMA接收缓冲区
//  */
// uint8_t rx_buffer[128] = {0};

// /**
//  * @brief DMA发送缓冲区（硬件DMA缓冲）
//  */
// static uint8_t uart_dma_tx_buf[UART_TX_BUF_SIZE];

// /**
//  * @brief 环形发送缓冲区（非阻塞printf支持）
//  */
// static uint8_t tx_ring_buf[UART_TX_BUF_SIZE] = {0};
// static volatile uint16_t tx_write_idx = 0;   // 生产者写
// static volatile uint16_t tx_read_idx  = 0;   // 消费者读
// static volatile uint8_t  tx_busy = 0;        // DMA正在发送

// /**
//  * @brief 命令接收缓冲区
//  */
// static char usart_rx_buffer[64] = {0};
// static volatile uint16_t usart_rx_index = 0;
// static volatile uint8_t command_ready = 0;
// static volatile uint32_t rx_count = 0;

// // ====================== 环形缓冲区工具函数 ======================

// /**
//  * @brief 判断环形缓冲区是否为空
//  */
// static inline uint8_t ringbuf_is_empty(void)
// {
//     return tx_write_idx == tx_read_idx;
// }

// /**
//  * @brief 判断环形缓冲区是否已满（牺牲1字节判满）
//  */
// static inline uint8_t ringbuf_is_full(void)
// {
//     return ((tx_write_idx + 1) % UART_TX_BUF_SIZE) == tx_read_idx;
// }

// /**
//  * @brief 计算环形缓冲区中可读数据量
//  */
// static uint16_t ringbuf_available(void)
// {
//     uint16_t w = tx_write_idx, r = tx_read_idx;
//     return (w >= r) ? (w - r) : (UART_TX_BUF_SIZE - r + w);
// }

// /**
//  * @brief 计算环形缓冲区中剩余空间（最大可写入字节数）
//  */
// /*
// static uint16_t ringbuf_free_space(void)
// {
//     // 总空间 = SIZE - 1（1字节用于判满）
//     return (UART_TX_BUF_SIZE - 1) - ringbuf_available();
// }
// */

// /**
//  * @brief 环形缓冲区写入1字节（内部调用，**调用前需关中断**）
//  * @return 1 成功，0 失败（满）
//  */
// static uint8_t ringbuf_write_byte(uint8_t byte)
// {
//     if (ringbuf_is_full()) {
//         return 0; // 满，丢弃（可改策略）
//     }
//     tx_ring_buf[tx_write_idx] = byte;
//     tx_write_idx = (tx_write_idx + 1) % UART_TX_BUF_SIZE;
//     return 1;
// }

// /**
//  * @brief 启动DMA传输（从环形缓冲区搬运数据到DMA硬件缓冲并启动发送）
//  * @note 必须在关中断或确定无竞争时调用（如 ISR 或关中断后）
//  */
// static void uart_start_dma_transfer(void)
// {
//     if (tx_busy || ringbuf_is_empty()) {
//         return;
//     }

//     uint16_t len = 0;
//     uint16_t r = tx_read_idx;
//     uint16_t w = tx_write_idx;

//     if (w > r) {
//         len = w - r;
//         memcpy(uart_dma_tx_buf, &tx_ring_buf[r], len);
//     } else if (w < r) {
//         uint16_t part1 = UART_TX_BUF_SIZE - r;
//         memcpy(uart_dma_tx_buf, &tx_ring_buf[r], part1);
//         len = part1;
//         if (w > 0) {
//             memcpy(&uart_dma_tx_buf[part1], tx_ring_buf, w);
//             len += w;
//         }
//     } else {
//         return; // empty
//     }

//     // 更新读指针（必须在 memcpy 后）
//     tx_read_idx = (r + len) % UART_TX_BUF_SIZE;

//     // 安全禁用DMA（先清除标志，再禁用）
//     DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
//     DMA_Cmd(DMA2_Stream7, DISABLE);
//     while (DMA2_Stream7->CR & DMA_SxCR_EN); // 等待真正禁用

//     // 设置新传输长度并启动
//     DMA_SetCurrDataCounter(DMA2_Stream7, len);
//     DMA_Cmd(DMA2_Stream7, ENABLE);
//     USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
//     tx_busy = 1;
// }

// // ====================== 公共函数 ======================

// void printf_array(uint8_t *arr, uint16_t len)
// {
//     for (uint16_t i = 0; i < len; i++) {
//         printf("%#x ", arr[i]);
//     }
//     printf("\n");
// }

// void USART1_IRQHandler(void)
// {
//     if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET) {
//         volatile uint16_t temp;
//         temp = USART1->SR;
//         temp = USART1->DR;

//         DMA_Cmd(DMA2_Stream5, DISABLE);
//         uint16_t recv_len = sizeof(rx_buffer) - DMA_GetCurrDataCounter(DMA2_Stream5);
//         if (recv_len == 0) recv_len = sizeof(rx_buffer);

//         for (uint16_t i = 0; i < recv_len; i++) {
//             uint8_t ch = rx_buffer[i];
//             rx_count++;

//             if (ch == '\r' || ch == '\n') {
//                 if (usart_rx_index > 0) {
//                     usart_rx_buffer[usart_rx_index] = '\0';
//                     command_ready = 1;
//                     usart_rx_index = 0;
//                 }
//             } else if (usart_rx_index < sizeof(usart_rx_buffer) - 1) {
//                 usart_rx_buffer[usart_rx_index++] = ch;
//             }
//         }

//         DMA_SetCurrDataCounter(DMA2_Stream5, sizeof(rx_buffer));
//         DMA_Cmd(DMA2_Stream5, ENABLE);
//     }
// }

// void DMA2_Stream7_IRQHandler(void)
// {
//     if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET) {
//         DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
        
//         DMA_Cmd(DMA2_Stream7, DISABLE);
//         USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
//         tx_busy = 0;

//     // 关中断保护：防止写入与读取冲突
//     taskENTER_CRITICAL();
//     if (!ringbuf_is_empty()) {
//         uart_start_dma_transfer();
//     }
//     taskEXIT_CRITICAL();
//     }
// }

// static void usart1_init(void)
// {
//     GPIO_InitTypeDef GPIO_InitStruct;
//     USART_InitTypeDef USART_InitStruct;
//     NVIC_InitTypeDef NVIC_InitStruct;

//     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

//     GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
//     GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
//     GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
//     GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//     GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // RX建议上拉
//     GPIO_Init(GPIOA, &GPIO_InitStruct);

//     GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
//     GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

//     USART_InitStruct.USART_BaudRate = 115200;
//     USART_InitStruct.USART_WordLength = USART_WordLength_8b;
//     USART_InitStruct.USART_StopBits = USART_StopBits_1;
//     USART_InitStruct.USART_Parity = USART_Parity_No;
//     USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//     USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//     USART_Init(USART1, &USART_InitStruct);

//     USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

//     NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
//     NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
//     NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
//     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStruct);

//     USART_Cmd(USART1, ENABLE);
// }

// void usart1_dma_rx_init(void)
// {
//     DMA_InitTypeDef DMA_InitStruct;
//     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
//     usart1_init();

//     DMA_StructInit(&DMA_InitStruct);
//     DMA_InitStruct.DMA_Channel = DMA_Channel_4;
//     DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
//     DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)rx_buffer;
//     DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
//     DMA_InitStruct.DMA_BufferSize = sizeof(rx_buffer);
//     DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//     DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
//     DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//     DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//     DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
//     DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
//     DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
//     DMA_Init(DMA2_Stream5, &DMA_InitStruct);

//     DMA_Cmd(DMA2_Stream5, ENABLE);
//     USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
// }

// void usart1_dma_tx_init(void)
// {
//     DMA_InitTypeDef DMA_InitStruct;
//     NVIC_InitTypeDef NVIC_InitStruct;

//     DMA_StructInit(&DMA_InitStruct);
//     DMA_InitStruct.DMA_Channel = DMA_Channel_4;
//     DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
//     DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)uart_dma_tx_buf;
//     DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
//     DMA_InitStruct.DMA_BufferSize = 0;
//     DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//     DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
//     DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//     DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//     DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
//     DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
//     DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
//     DMA_Init(DMA2_Stream7, &DMA_InitStruct);

//     DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
    
//     NVIC_InitStruct.NVIC_IRQChannel = DMA2_Stream7_IRQn;
//     NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 4;
//     NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
//     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStruct);

//     DMA_Cmd(DMA2_Stream7, DISABLE);
// }

// void Usart1_Send_DMA(uint8_t *data, uint16_t len)
// {
//     taskENTER_CRITICAL();
//     for (uint16_t i = 0; i < len; i++) {
//         // 不死等：若满则丢弃（或可加超时）
//         if (!ringbuf_write_byte(data[i])) {
//             // 可选：记录丢包统计
//         }
//     }
//     taskEXIT_CRITICAL();

//     if (!tx_busy) {
//         taskENTER_CRITICAL();
//         uart_start_dma_transfer();
//         taskEXIT_CRITICAL();
//     }
// }

// void Usart1_Send_String(char *string)
// {
//     printf("%s", string);
// }

// void Usart1_send_bytes(uint8_t *data, uint16_t len)
// {
//     if (data && len > 0) {
//         Usart1_Send_DMA(data, len);
//     }
// }

// uint32_t get_usart_rx_count(void)
// {
//     return rx_count;
// }

// uint8_t is_command_ready(void)
// {
//     return command_ready;
// }

// uint8_t Usart1_Receive_String(char *buffer, uint16_t size)
// {
//     if (command_ready && buffer && size > 0) {
//         uint16_t len = strlen(usart_rx_buffer);
//         if (len < size) {
//             strcpy(buffer, usart_rx_buffer);
//             command_ready = 0;
//             // 更高效：只清已用部分
//             memset(usart_rx_buffer, 0, len + 1);
//             return 1;
//         }
//     }
//     return 0;
// }


// //对收到的指令判别
// void Process_Usart_Command(void)
// {
//     char cmd[64];
//     if (Usart1_Receive_String(cmd, sizeof(cmd))) {
//         for (int i = 0; cmd[i]; i++) {
//             if (cmd[i] >= 'A' && cmd[i] <= 'Z')
//                 cmd[i] += 32;
//         }

//         if (strcmp(cmd, "led0 on") == 0) {
//             LED0 = 0;
//             printf("LED0 ON\r\n");
//         } else if (strcmp(cmd, "led0 off") == 0) {
//             LED0 = 1;
//             printf("LED0 OFF\r\n");
//         } else if (strcmp(cmd, "led1 on") == 0) {
//             LED1 = 0;
//             printf("LED1 ON\r\n");
//         } else if (strcmp(cmd, "led1 off") == 0) {
//             LED1 = 1;
//             printf("LED1 OFF\r\n");
//         } else if (strcmp(cmd, "led2 on") == 0) {
//             LED2 = 0;
//             printf("LED2 ON\r\n");
//         } else if (strcmp(cmd, "led2 off") == 0) {
//             LED2 = 1;
//             printf("LED2 OFF\r\n");
//         } else if (strcmp(cmd, "led3 on") == 0) {
//             LED3 = 0;
//             printf("LED3 ON\r\n");
//         } else if (strcmp(cmd, "led3 off") == 0) {
//             LED3 = 1;
//             printf("LED3 OFF\r\n");
//         } else if (strcmp(cmd, "all on") == 0) {
//             LED0 = LED1 = LED2 = LED3 = 0;
//             printf("ALL LEDS ON\r\n");
//         } else if (strcmp(cmd, "all off") == 0) {
//             LED0 = LED1 = LED2 = LED3 = 1;
//             printf("ALL LEDS OFF\r\n");
//         } else if (strcmp(cmd, "help") == 0) {
//             printf("Commands:\r\n"
//                    "led0/1/2/3 on/off - Control individual LED\r\n"
//                    "all on/off - Control all LEDs\r\n"
//                    "0c/1c/2c/3c - Toggle LED state\r\n");
//         } else if (strcmp(cmd, "0c") == 0) {
//             LED0 = !LED0;
//             printf("LED0 toggled\r\n");
//         } else if (strcmp(cmd, "1c") == 0) {
//             LED1 = !LED1;
//             printf("LED1 toggled\r\n");
//         } else if (strcmp(cmd, "2c") == 0) {
//             LED2 = !LED2;
//             printf("LED2 toggled\r\n");
//         } else if (strcmp(cmd, "3c") == 0) {
//             LED3 = !LED3;
//             printf("LED3 toggled\r\n");
//         } else if (strcmp(cmd, "get time") == 0) {
            
//            RTC_Date_Get();
//         } else {
//             printf("Unknown command. Type 'help'\r\n");
//         }
//     }
// }

// void debug_init(void)
// {
//     usart1_dma_rx_init();
//     usart1_dma_tx_init();
// }

// int fputc(int ch, FILE *f)
// {
//     uint8_t byte = (uint8_t)ch;

//     // 自动补 \r（常见于串口终端）
//     if (byte == '\n') {
//         taskENTER_CRITICAL();
//         ringbuf_write_byte('\r');
//         taskEXIT_CRITICAL();
//     }

//     taskENTER_CRITICAL();
//     uint8_t ok = ringbuf_write_byte(byte);
//     taskEXIT_CRITICAL();

//     if (!ok) {
//         // 可选：记录发送失败
//         return -1;
//     }

//     if (!tx_busy) {
//         taskENTER_CRITICAL();
//         uart_start_dma_transfer();
//         taskEXIT_CRITICAL();
//     }

//     return ch;
// }

// void uart_tx_task(void)
// {
//     if (!tx_busy && !ringbuf_is_empty()) {
//         taskENTER_CRITICAL();
//         uart_start_dma_transfer();
//         taskEXIT_CRITICAL();
//     }
// }

// uint16_t uart_get_tx_buf_usage(void)
// {
//     return ringbuf_available();
// }
