/**
 * @file led.h
 * @brief STM32F4xx LED驱动模块
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 * 
 * 本文件提供STM32F4系列微控制器的LED驱动功能，
 * 包括LED初始化、控制和状态读取等功能。
 * 采用分层架构设计，依赖底层GPIO模块。
 */

#ifndef _LED_H_
#define _LED_H_

#include "sys.h"
#include "gpio.h"

// ==================================
// LED操作错误码定义
// ==================================

/**
 * @defgroup LED_Error_Codes LED操作错误码定义
 * @{
 */
#define LED_OK              0   ///< 操作成功
#define LED_PARAM_ERROR     1   ///< 参数错误
#define LED_INVALID_LED     2   ///< 无效LED编号
/** @} */

// ==================================
// LED初始化函数声明
// ==================================

/**
 * @brief LED GPIO初始化函数
 * @param LED_PIN LED引脚
 * @param LED_Port LED所在GPIO端口
 * @return 错误码：LED_OK-成功，其他-失败
 * @note 配置为高速输出模式，推挽输出，无上下拉
 */
int8_t LED_Init_Single(uint32_t LED_Pin, GPIO_TypeDef *LED_Port);

/**
 * @brief 按编号初始化单个LED
 * @param led_num LED编号(0-3)
 * @return 错误码：LED_OK-成功，LED_PARAM_ERROR-参数错误，其他-失败
 * @note 根据编号初始化对应的LED:
 *       - 0: 初始化LED0 (PF9)
 *       - 1: 初始化LED1 (PF10)
 *       - 2: 初始化LED2 (PE13)
 *       - 3: 初始化LED3 (PE14)
 */
int8_t LED_Init_ByNumber(uint8_t led_num);

/**
 * @brief 初始化所有LED
 * @return 错误码：LED_OK-成功，其他-失败
 */
int8_t LED_Init(void);

// ==================================
// LED控制函数声明
// ==================================

/**
 * @brief 设置LED状态
 * @param led_num LED编号(0-3)
 * @param state LED状态：0-亮，1-灭
 * @return 错误码：LED_OK-成功，LED_INVALID_LED-无效LED编号
 * @note 使用位带操作控制LED，硬件设计：低电平点亮，高电平熄灭
 */
int8_t LED_Set(uint8_t led_num, uint8_t state);

/**
 * @brief 切换LED状态
 * @param led_num LED编号(0-3)
 * @return 错误码：LED_OK-成功，LED_INVALID_LED-无效LED编号
 * @note 如果LED当前是亮的，则熄灭；如果是灭的，则点亮
 */
int8_t LED_Toggle(uint8_t led_num);

/**
 * @brief 设置所有LED状态
 * @param state LED状态：0-全亮，1-全灭
 * @return 错误码：LED_OK-成功
 */
int8_t LED_Set_All(uint8_t state);

// ==================================
// LED状态读取函数声明
// ==================================

/**
 * @brief 读取LED状态
 * @param led_num LED编号(0-3)
 * @param state 输出参数，指向存储LED状态的指针
 * @return 错误码：LED_OK-成功，LED_INVALID_LED-无效LED编号，LED_PARAM_ERROR-参数错误
 * @note 使用位带操作读取LED当前状态
 */
int8_t LED_Get_State(uint8_t led_num, uint8_t *state);

/**
 * @brief 检查LED是否点亮
 * @param led_num LED编号(0-3)
 * @return 1-点亮，0-熄灭，参数错误返回0
 */
uint8_t LED_Is_On(uint8_t led_num);

/**
 * @brief 检查LED是否熄灭
 * @param led_num LED编号(0-3)
 * @return 1-熄灭，0-点亮，参数错误返回0
 */
uint8_t LED_Is_Off(uint8_t led_num);
// 函数声明
void ls_interruptible(void);
void ysysled(void);

#endif /* _LED_H_ */
