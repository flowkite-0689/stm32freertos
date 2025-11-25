#ifndef RFID_H
#define RFID_H

/********************这些宏需要你填充************************* */
#include "rfid_uart.h"
#include "delay.h"
// 配置接收的数据缓冲区
#define RFID_Init(rx_buffer, len)   RFID_Uart2_Rx_DMA_Init(rx_buffer, len)

// 主机发送指定长度到rfid
#define RFID_SendBytes(data, len)   RFID_Uart2_SendBytes(data, len)

// 主机发送数据后收到rfid返回的数据长度
#define RFID_GetFrame_len()          RFID_Uart2_Recv_Len() 

// 延时
#define RFID_delay_ms(ms) 			 delay_ms(ms)
/***************************end************************** */

#define RFID_FRAME_MAX_LEN		128		// rfid发送数据帧最大长度
#define RFID_TIMEOUT			3		// rfid数据帧接收超时时间3S
#define RFID_ETX				0X03	// 结束符
#define S50_ATQ					0x0004	// 请求时，S50卡ATQ
#define S50_SAK					0x08	// 一级防碰撞选择时，S50卡SAK

void RFID_demo(void);

#endif
