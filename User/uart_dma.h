#ifndef UART_DMA_H
#define UART_DMA_H

#include "stm32f4xx.h"
#include <stdio.h>

void usart1_dma_rx_init(void);
void usart1_dma_tx_init(void);
void Usart1_Send_String(char *string);
void Usart1_Send_DMA(uint8_t *data, uint16_t len);

// 从debug.h移植过来的函数声明
uint32_t get_usart_rx_count(void);
uint8_t is_command_ready(void);
void debug_init(void);
uint8_t Usart1_Receive_String(char *buffer, uint16_t size);
void Process_Usart_Command(void);

#endif