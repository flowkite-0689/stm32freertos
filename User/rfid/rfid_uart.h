#ifndef RFID_UART_H
#define RFID_UART_H

#include "stm32f4xx.h"

void RFID_Uart2_Rx_DMA_Init(uint8_t *rx_buffer, uint16_t len);
void RFID_Uart2_SendBytes(uint8_t *data, uint16_t len);
uint16_t RFID_Uart2_Recv_Len(void);

#endif
