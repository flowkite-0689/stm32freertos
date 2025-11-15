/**
 * @file gpio.h
 * @brief STM32F4xx GPIO底层操作模块
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 * 
 * 本文件提供STM32F4系列微控制器的底层GPIO操作功能，
 * 包括GPIO时钟使能、GPIO初始化等基础功能。
 * 采用分层架构设计，提供带参数验证的可靠接口。
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <stddef.h>

// ==================================
// 错误码定义
// ==================================

/**
 * @defgroup GPIO_Error_Codes GPIO操作错误码定义
 * @{
 */
#define GPIO_OK           0   ///< 操作成功
#define GPIO_PARAM_ERROR  1   ///< 参数错误
#define GPIO_PORT_ERROR   2   ///< 无效GPIO端口
/** @} */

// ==================================
// GPIO时钟使能函数声明
// ==================================

/**
 * @brief 使能指定GPIO端口的时钟
 * @param GPIOx GPIO端口指针（如GPIOA、GPIOB等）
 * @return 错误码：GPIO_OK-成功，GPIO_PORT_ERROR-无效端口
 * @note 如果端口指针无效，函数将返回错误码
 */
int8_t GPIO_Clock_Enable(GPIO_TypeDef *GPIOx);

// ==================================
// GPIO初始化函数声明
// ==================================

/**
 * @brief 带参数验证的GPIO通用初始化函数
 * @param GPIOx GPIO端口（如GPIOA、GPIOB等）
 * @param GPIO_Pin GPIO引脚（如GPIO_Pin_0、GPIO_Pin_1等）
 * @param GPIO_Mode GPIO模式（输入、输出、复用、模拟）
 * @param GPIO_Speed GPIO速度（2MHz、25MHz、50MHz、100MHz）
 * @param GPIO_OType GPIO输出类型（推挽、开漏）
 * @param GPIO_PuPd GPIO上下拉配置（上拉、下拉、无上下拉）
 * @return 错误码：GPIO_OK-成功，GPIO_PARAM_ERROR-参数错误，GPIO_PORT_ERROR-无效端口
 * @note 函数会根据传入的GPIOx参数自动使能对应的AHB1时钟
 * @note 使用本函数前确保所有参数已正确设置
 */
int8_t GPIO_Init_WithCheck(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode,
                          GPIOSpeed_TypeDef GPIO_Speed, GPIOOType_TypeDef GPIO_OType,
                          GPIOPuPd_TypeDef GPIO_PuPd);

// ==================================
// GPIO操作函数声明
// ==================================

/**
 * @brief 设置GPIO引脚输出电平
 * @param GPIOx GPIO端口
 * @param GPIO_Pin 引脚位
 * @param state 输出状态：0-低电平，1-高电平
 * @return 错误码：GPIO_OK-成功，GPIO_PARAM_ERROR-参数错误，GPIO_PORT_ERROR-无效端口
 */
int8_t GPIO_Set_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t state);

/**
 * @brief 读取GPIO引脚输入电平
 * @param GPIOx GPIO端口
 * @param GPIO_Pin 引脚位
 * @param state 输出参数，指向存储读取状态的指针
 * @return 错误码：GPIO_OK-成功，GPIO_PARAM_ERROR-参数错误，GPIO_PORT_ERROR-无效端口
 */
int8_t GPIO_Read_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t *state);

#endif /* _GPIO_H_ */
