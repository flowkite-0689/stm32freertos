/**
 * @file uart_dma.h
 * @brief STM32串口DMA驱动模块头文件
 * @details 声明串口DMA接收/发送、命令处理、调试输出等接口
 *          配合 uart_dma.c 使用，支持非阻塞 printf、IDLE中断帧检测
 * @author Developer
 * @date 2024
 */

#ifndef __UART_DMA_H
#define __UART_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdio.h>

// =============================================================================
// 配置宏（可在外层定义覆盖，默认值合理）
// =============================================================================
#ifndef UART_TX_BUF_SIZE
#define UART_TX_BUF_SIZE    256   ///< 环形发送缓冲区大小（建议 ≥ 128），注意：实际可用空间为 UART_TX_BUF_SIZE - 1
#endif

// =============================================================================
// 公共函数声明
// =============================================================================

/**
 * @brief 初始化USART1 DMA接收与发送（兼容旧接口）
 * @note 等价于先调用 usart1_dma_rx_init()，再调用 usart1_dma_tx_init()
 */
void debug_init(void);

/**
 * @brief 初始化USART1 DMA接收（IDLE中断 + 循环DMA）
 */
void usart1_dma_rx_init(void);

/**
 * @brief 初始化USART1 DMA发送（正常模式 + TC中断）
 */
void usart1_dma_tx_init(void);

/**
 * @brief 使用DMA发送原始数据（非阻塞，内部使用环形缓冲）
 * @param data 指向待发送数据的指针
 * @param len  待发送数据长度（字节）
 */
void Usart1_Send_DMA(uint8_t *data, uint16_t len);

/**
 * @brief 通过串口发送字符串（推荐直接使用 printf）
 * @param string 以 '\0' 结尾的字符串
 */
void Usart1_Send_String(char *string);

/**
 * @brief 检查是否有完整命令就绪（以 \r 或 \n 结尾）
 * @return 1: 有命令待处理；0: 无命令
 */
uint8_t is_command_ready(void);

/**
 * @brief 获取已就绪的命令字符串（成功后自动清除标志）
 * @param buffer 用户提供的缓冲区（至少 size 字节）
 * @param size   缓冲区大小
 * @return 1: 成功获取；0: 失败（无命令或缓冲区太小）
 */
uint8_t Usart1_Receive_String(char *buffer, uint16_t size);

/**
 * @brief 处理已接收的串口命令（LED控制等）
 * @note 需在主循环中定期调用
 */
void Process_Usart_Command(void);

/**
 * @brief 获取累计接收到的字节数（用于调试/统计）
 * @return uint32_t 接收字节总数
 */
uint32_t get_usart_rx_count(void);

/**
 * @brief 以十六进制格式打印数组内容
 * @param arr 待打印数组指针
 * @param len 数组长度
 */
void printf_array(uint8_t *arr, uint16_t len);

/**
 * @brief 后台发送任务（建议在主循环中调用，确保数据及时发出）
 * @note 若已使用 DMA 中断续发，此函数可省略；但保留可提高可靠性
 */
void uart_tx_task(void);

/**
 * @brief 获取当前发送环形缓冲区使用量（字节数）
 * @return uint16_t 已占用字节数
 */
uint16_t uart_get_tx_buf_usage(void);

// =============================================================================
// 标准库 printf 重定向支持（无需用户调用）
// =============================================================================
/**
 * @brief 重定向 printf 到 USART1（非阻塞，支持 \n → \r\n 自动转换）
 * @note 编译器自动链接，用户直接调用 printf 即可
 */
int fputc(int ch, FILE *f);

// =============================================================================
// 中断服务函数声明（由启动文件调用，用户无需手动调用）
// =============================================================================
void USART1_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_DMA_H */

