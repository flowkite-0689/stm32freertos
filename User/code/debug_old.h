#ifndef DEBUG_H
#define DEBUG_H

#include "sys.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>
extern QueueHandle_t uart_rx_queue ;
extern QueueHandle_t uart_tx_queue ;
void uart_queue_init(void);
void uart_queue_process_task(void *pvParameters);   // 你要在 main 里创建这个任务
uint32_t uart_get_rx_count(void);


void debug_init(void);
void Usart1_Send_String(char *str);
void Usart1_send_bytes(uint8_t *data, uint16_t len);
#endif
