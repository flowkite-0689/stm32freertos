/**
 * @file myInit.h
 * @brief STM32F4xx GPIO初始化及延时函数封装库
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.11.8
 *
 * 本文件提供了STM32F4系列微控制器的GPIO初始化、延时函数等
 * 基础功能的封装，简化了硬件初始化流程。
 */

#ifndef _MYINIT_H_
#define _MYINIT_H_

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/**
 * @defgroup LED_Definitions LED引脚定义
 * @{
 */
#define LED0_PIN GPIO_Pin_9  ///< LED0引脚定义
#define LED0_PORT GPIOF      ///< LED0端口定义
#define LED1_PIN GPIO_Pin_10 ///< LED1引脚定义
#define LED1_PORT GPIOF      ///< LED1端口定义
#define LED2_PIN GPIO_Pin_13 ///< LED2引脚定义
#define LED2_PORT GPIOE      ///< LED2端口定义
#define LED3_PIN GPIO_Pin_14 ///< LED3引脚定义
#define LED3_PORT GPIOE      ///< LED3端口定义
/** @} */

/**
 * @defgroup KEY_Definitions 按键引脚定义
 * @{
 */
#define KEY0_PIN GPIO_Pin_0 ///< 按键0引脚定义
#define KEY0_PORT GPIOA     ///< 按键0端口定义
#define KEY1_PIN GPIO_Pin_2 ///< 按键1引脚定义
#define KEY1_PORT GPIOE     ///< 按键1端口定义
#define KEY2_PIN GPIO_Pin_3 ///< 按键2引脚定义
#define KEY2_PORT GPIOE     ///< 按键2端口定义
#define KEY3_PIN GPIO_Pin_4 ///< 按键3引脚定义
#define KEY3_PORT GPIOE     ///< 按键3端口定义
/** @} */

/**
 * @defgroup BEEP_Definitions 蜂鸣器引脚定义
 * @{
 */
#define BEEP0_PIN GPIO_Pin_8
#define BEEP0_PORT GPIOF
/** @} */

/**
 * @brief 毫秒级延时函数
 * @param ms 延时时间（单位：毫秒）
 * @note 基于系统时钟的软件延时，精度较低，适用于非精确延时场景
 */
void Delay_ms(uint32_t ms);

/**
 * @brief GPIO通用初始化函数
 * @param GPIOx GPIO端口（如GPIOA、GPIOB等）
 * @param GPIO_Pin GPIO引脚（如GPIO_Pin_0、GPIO_Pin_1等）
 * @param GPIO_Mode GPIO模式（输入、输出、复用、模拟）
 * @param GPIO_Speed GPIO速度（2MHz、25MHz、50MHz、100MHz）
 * @param GPIO_OType GPIO输出类型（推挽、开漏）
 * @param GPIO_PuPd GPIO上下拉配置（上拉、下拉、无上下拉）
 * @note 函数会自动根据GPIOx参数使能对应的时钟
 */
void GPIO_MyInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode,
                 GPIOSpeed_TypeDef GPIO_Speed, GPIOOType_TypeDef GPIO_OType,
                 GPIOPuPd_TypeDef GPIO_PuPd);

/**
 * @brief 按键GPIO初始化函数
 * @param KEY_PIN 按键引脚
 * @param GPIOx 按键所在GPIO端口
 * @note 配置为高速输入模式，推挽输出，无上下拉
 */
void KEY_Init(uint32_t KEY_PIN, GPIO_TypeDef *GPIOx);

/**
 * @brief LED GPIO初始化函数
 * @param LED_PIN LED引脚
 * @param GPIOx LED所在GPIO端口
 * @note 配置为高速输出模式，推挽输出，无上下拉
 */
void LED_Init(uint32_t LED_PIN, GPIO_TypeDef *GPIOx);

/**
 * @brief 蜂鸣器GPIO初始化函数
 * @param BEEP_PIN 蜂鸣器引脚
 * @param GPIOx 蜂鸣器所在GPIO端口
 * @note 配置为50MHz输出模式，推挽输出，无上下拉
 */
void BEEP_Init(uint32_t BEEP_PIN, GPIO_TypeDef *GPIOx);


/**
 * @brief 按键GPIO初始化二次封装函数
 * @param x 按键编号(0-3)
 * @note 根据编号初始化对应的按键GPIO:
 *       - 0: 初始化KEY0 (PA0)
 *       - 1: 初始化KEY1 (PE2)
 *       - 2: 初始化KEY2 (PE3)
 *       - 3: 初始化KEY3 (PE4)
 * @note 其他值则不进行任何操作
 */
void KEY_Initx(uint32_t x);

/**
 * @brief LED GPIO初始化二次封装函数
 * @param x LED编号(0-3)
 * @note 根据编号初始化对应的LED GPIO:
 *       - 0: 初始化LED0 (PF9)
 *       - 1: 初始化LED1 (PF10)
 *       - 2: 初始化LED2 (PE13)
 *       - 3: 初始化LED3 (PE14)
 * @note 其他值则不进行任何操作
 */
void LED_Initx(uint32_t x);

/**
 * @brief 蜂鸣器GPIO初始化二次封装函数
 * @param x 蜂鸣器编号
 * @note 当前仅支持编号0:
 *       - 0: 初始化BEEP0 (PF8)
 * @note 其他值则不进行任何操作
 * @note 预留接口，可扩展支持多个蜂鸣器
 */
void BEEP_Initx(uint32_t x);

#endif

