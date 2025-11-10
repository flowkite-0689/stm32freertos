/**
 * @file beep.h
 * @brief STM32F4xx 蜂鸣器驱动模块
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 * 
 * 本文件提供STM32F4系列微控制器的蜂鸣器驱动功能，
 * 包括蜂鸣器初始化、控制和状态读取等功能。
 * 采用分层架构设计，依赖底层GPIO模块和延时模块。
 */

#ifndef _BEEP_H_
#define _BEEP_H_

#include "sys.h"
#include "gpio.h"
#include "delay.h"

// ==================================
// 蜂鸣器操作错误码定义
// ==================================

/**
 * @defgroup BEEP_Error_Codes 蜂鸣器操作错误码定义
 * @{
 */
#define BEEP_OK             0   ///< 操作成功
#define BEEP_PARAM_ERROR    1   ///< 参数错误
#define BEEP_INVALID_BEEP   2   ///< 无效蜂鸣器编号
/** @} */

// ==================================
// 蜂鸣器初始化函数声明
// ==================================

/**
 * @brief 蜂鸣器GPIO初始化函数
 * @param BEEP_Pin 蜂鸣器引脚
 * @param BEEP_Port 蜂鸣器所在GPIO端口
 * @return 错误码：BEEP_OK-成功，其他-失败
 * @note 配置为50MHz输出模式，推挽输出，无上下拉
 */
int8_t BEEP_Init_Single(uint32_t BEEP_Pin, GPIO_TypeDef *BEEP_Port);

/**
 * @brief 按编号初始化单个蜂鸣器
 * @param beep_num 蜂鸣器编号
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号，其他-失败
 * @note 当前仅支持编号0:
 *       - 0: 初始化BEEP0 (PF8)
 */
int8_t BEEP_Init_ByNumber(uint8_t beep_num);

/**
 * @brief 初始化所有蜂鸣器
 * @return 错误码：BEEP_OK-成功，其他-失败
 */
int8_t BEEP_Init(void);

// ==================================
// 蜂鸣器控制函数声明
// ==================================

/**
 * @brief 设置蜂鸣器状态
 * @param beep_num 蜂鸣器编号
 * @param state 蜂鸣器状态：1-开启，0-关闭
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号
 * @note 使用位带操作控制蜂鸣器
 */
int8_t BEEP_Set(uint8_t beep_num, uint8_t state);

/**
 * @brief 切换蜂鸣器状态
 * @param beep_num 蜂鸣器编号
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号
 * @note 如果蜂鸣器当前是开启的，则关闭；如果是关闭的，则开启
 */
int8_t BEEP_Toggle(uint8_t beep_num);

/**
 * @brief 蜂鸣器发声
 * @param beep_num 蜂鸣器编号
 * @param duration_ms 持续时间（毫秒）
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号，BEEP_PARAM_ERROR-参数错误
 * @note 控制蜂鸣器发声指定时间后自动停止
 */
int8_t BEEP_Buzz(uint8_t beep_num, uint32_t duration_ms);

// ==================================
// 蜂鸣器状态读取函数声明
// ==================================

/**
 * @brief 读取蜂鸣器状态
 * @param beep_num 蜂鸣器编号
 * @param state 输出参数，指向存储蜂鸣器状态的指针
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号，BEEP_PARAM_ERROR-参数错误
 * @note 使用位带操作读取蜂鸣器当前状态
 */
int8_t BEEP_Get_State(uint8_t beep_num, uint8_t *state);

/**
 * @brief 检查蜂鸣器是否开启
 * @param beep_num 蜂鸣器编号
 * @return 1-开启，0-关闭，参数错误返回0
 */
uint8_t BEEP_Is_On(uint8_t beep_num);

/**
 * @brief 检查蜂鸣器是否关闭
 * @param beep_num 蜂鸣器编号
 * @return 1-关闭，0-开启，参数错误返回0
 */
uint8_t BEEP_Is_Off(uint8_t beep_num);

#endif /* _BEEP_H_ */
