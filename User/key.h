/**
 * @file key.h
 * @brief STM32F4xx 按键驱动模块
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 * 
 * 本文件提供STM32F4系列微控制器的按键驱动功能，
 * 包括按键初始化、状态读取和消抖等功能。
 * 采用分层架构设计，依赖底层GPIO模块和延时模块。
 */

#ifndef _KEY_H_
#define _KEY_H_

#include "sys.h"
#include "gpio.h"
#include "delay.h"

// ==================================
// 按键操作错误码定义
// ==================================

/**
 * @defgroup KEY_Error_Codes 按键操作错误码定义
 * @{
 */
#define KEY_OK              0   ///< 操作成功
#define KEY_PARAM_ERROR     1   ///< 参数错误
#define KEY_INVALID_KEY     2   ///< 无效按键编号
#define KEY_NONE_PRESSED    0xFF ///< 无按键按下
/** @} */

// ==================================
// 按键初始化函数声明
// ==================================

/**
 * @brief 按键GPIO初始化函数
 * @param KEY_PIN 按键引脚
 * @param KEY_Port 按键所在GPIO端口
 * @return 错误码：KEY_OK-成功，其他-失败
 * @note 配置为高速输入模式，推挽输出，无上下拉
 */
int8_t KEY_Init_Single(uint32_t KEY_Pin, GPIO_TypeDef *KEY_Port);

/**
 * @brief 按编号初始化单个按键
 * @param key_num 按键编号(0-3)
 * @return 错误码：KEY_OK-成功，KEY_INVALID_KEY-无效按键编号，其他-失败
 * @note 根据编号初始化对应的按键:
 *       - 0: 初始化KEY0 (PA0)
 *       - 1: 初始化KEY1 (PE2)
 *       - 2: 初始化KEY2 (PE3)
 *       - 3: 初始化KEY3 (PE4)
 */
int8_t KEY_Init_ByNumber(uint8_t key_num);

/**
 * @brief 初始化所有按键
 * @return 错误码：KEY_OK-成功，其他-失败
 */
int8_t KEY_Init(void);

// ==================================
// 按键状态读取函数声明
// ==================================

/**
 * @brief 读取单个按键状态
 * @param key_num 按键编号(0-3)
 * @param state 输出参数，指向存储按键状态的指针
 * @return 错误码：KEY_OK-成功，KEY_INVALID_KEY-无效按键编号，KEY_PARAM_ERROR-参数错误
 * @note 按键硬件设计为下拉模式，按下时为低电平(0)，释放时为高电平(1)
 */
int8_t KEY_Get_State(uint8_t key_num, uint8_t *state);

/**
 * @brief 检查按键是否按下
 * @param key_num 按键编号(0-3)
 * @return 1-按下，0-释放，参数错误返回0
 */
uint8_t KEY_Is_Pressed(uint8_t key_num);

/**
 * @brief 检查按键是否释放
 * @param key_num 按键编号(0-3)
 * @return 1-释放，0-按下，参数错误返回0
 */
uint8_t KEY_Is_Released(uint8_t key_num);

// ==================================
// 按键消抖函数声明
// ==================================

/**
 * @brief 按键消抖检测
 * @param key_num 按键编号(0-3)
 * @return 错误码：KEY_OK-确认按下，其他-未按下或抖动/参数错误
 * @note 进行延时消抖确认
 */
int8_t KEY_Debounce(uint8_t key_num);

/**
 * @brief 等待按键释放
 * @param key_num 按键编号(0-3)
 * @return 错误码：KEY_OK-成功，其他-参数错误
 * @note 阻塞等待按键释放
 */
int8_t KEY_WaitForRelease(uint8_t key_num);

// ==================================
// 按键检测函数声明
// ==================================

/**
 * @brief 获取按键状态（带消抖和等待释放）
 * @return 按键编号(0~3)，KEY_NONE_PRESSED表示无按键按下
 * @note 采用"快速检测+延时确认+等待释放"的处理逻辑
 */
uint8_t KEY_Read(void);

#endif /* _KEY_H_ */
