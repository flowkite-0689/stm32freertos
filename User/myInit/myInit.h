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

//位带操作
/*
位带别名地址 = 外设别名基地址 + (原始寄存器地址-外设基地址)*32 +  引脚的号数 * 4
*32 即 << 5 ,,*4 即 << 2
*/
#define BITBAND(addr, bit) (*(volatile unsigned long *)((0x42000000U + ((addr - 0x40000000U) << 5) + ((bit) << 2))))

// 一次封装
#define GPIO_OUT(PORT, PIN) BITBAND((uint32_t)&PORT->ODR, PIN)
#define GPIO_IN(PORT, PIN) BITBAND((uint32_t)&PORT->IDR, PIN)
#define GPIO_SET(PORT,PIN ) (PORT->BSRRL = (1UL << PIN)) //bit set reset low register 置1
#define GPIO_RST(PORT, PIN)     (PORT->BSRRH = (1UL << PIN)) //bit set reset high register 置0

//二次封装

/**
 * @defgroup LED_BitBand_Operations LED位带操作宏定义
 * @brief 提供LED控制的位带操作宏定义，支持直接对LED引脚进行位操作
 * @note LED硬件设计为低电平点亮，高电平熄灭
 * @{
 */
/**
 * @brief LED0控制宏
 * @param n LED状态：0-亮，1-灭
 * @note 使用位带操作直接控制LED0 (PF9)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED0(n) (n ? GPIO_SET(LED0_PORT, 9) : GPIO_RST(LED0_PORT, 9))

/**
 * @brief LED1控制宏
 * @param n LED状态：0-亮，1-灭
 * @note 使用位带操作直接控制LED1 (PF10)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED1(n) (n ? GPIO_SET(LED1_PORT, 10) : GPIO_RST(LED1_PORT, 10))

/**
 * @brief LED2控制宏
 * @param n LED状态：0-亮，1-灭
 * @note 使用位带操作直接控制LED2 (PE13)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED2(n) (n ? GPIO_SET(LED2_PORT, 13) : GPIO_RST(LED2_PORT, 13))

/**
 * @brief LED3控制宏
 * @param n LED状态：0-亮，1-灭
 * @note 使用位带操作直接控制LED3 (PE14)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED3(n) (n ? GPIO_SET(LED3_PORT, 14) : GPIO_RST(LED3_PORT, 14))

/**
 * @brief LED状态读取宏
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作直接读取LED0 (PF9)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED0_STATE() GPIO_IN(LED0_PORT, 9)

/**
 * @brief LED1状态读取宏
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作直接读取LED1 (PF10)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED1_STATE() GPIO_IN(LED1_PORT, 10)

/**
 * @brief LED2状态读取宏
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作直接读取LED2 (PE13)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED2_STATE() GPIO_IN(LED2_PORT, 13)

/**
 * @brief LED3状态读取宏
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作直接读取LED3 (PE14)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
#define LED3_STATE() GPIO_IN(LED3_PORT, 14)
/** @} */

/**
 * @defgroup BEEP_BitBand_Operations 蜂鸣器位带操作宏定义
 * @brief 提供蜂鸣器控制的位带操作宏定义，支持直接对蜂鸣器引脚进行位操作
 * @{
 */
/**
 * @brief 蜂鸣器0控制宏
 * @param n 蜂鸣器状态：1-开启，0-关闭
 * @note 使用位带操作直接控制蜂鸣器0 (PF8)
 */
#define BEEP0(n) (n ? GPIO_SET(BEEP0_PORT, 8) : GPIO_RST(BEEP0_PORT, 8))

/**
 * @brief 蜂鸣器0状态读取宏
 * @return 蜂鸣器状态：1-开启，0-关闭
 * @note 使用位带操作直接读取蜂鸣器0 (PF8)当前状态
 */
#define BEEP0_STATE() GPIO_IN(BEEP0_PORT, 8)
/** @} */

/**
 * @defgroup KEY_BitBand_Operations 按键位带操作宏定义
 * @brief 提供按键读取的位带操作宏定义，支持直接读取按键引脚状态
 * @note 按键硬件设计为下拉模式，按下时为低电平(0)，释放时为高电平(1)
 * @{
 */
/**
 * @brief 按键0状态读取宏
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作直接读取按键0 (PA0)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
#define KEY0_STATE() GPIO_IN(KEY0_PORT, 0)

/**
 * @brief 按键1状态读取宏
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作直接读取按键1 (PE2)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
#define KEY1_STATE() GPIO_IN(KEY1_PORT, 2)

/**
 * @brief 按键2状态读取宏
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作直接读取按键2 (PE3)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
#define KEY2_STATE() GPIO_IN(KEY2_PORT, 3)

/**
 * @brief 按键3状态读取宏
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作直接读取按键3 (PE4)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
#define KEY3_STATE() GPIO_IN(KEY3_PORT, 4)
/** @} */

/**
 * @defgroup LED_Control_Functions LED控制函数声明
 * @brief 提供LED控制的标准函数接口，支持函数式调用
 * @note LED硬件设计为低电平点亮，高电平熄灭
 * @{
 */
/**
 * @brief LED0控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED0 (PF9)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED0_Set(uint8_t state);

/**
 * @brief LED1控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED1 (PF10)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED1_Set(uint8_t state);

/**
 * @brief LED2控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED2 (PE13)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED2_Set(uint8_t state);

/**
 * @brief LED3控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED3 (PE14)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED3_Set(uint8_t state);

/**
 * @brief LED0状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED0 (PF9)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED0_GetState(void);

/**
 * @brief LED1状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED1 (PF10)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED1_GetState(void);

/**
 * @brief LED2状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED2 (PE13)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED2_GetState(void);

/**
 * @brief LED3状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED3 (PE14)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED3_GetState(void);
/** @} */

/**
 * @defgroup BEEP_Control_Functions 蜂鸣器控制函数声明
 * @brief 提供蜂鸣器控制的标准函数接口，支持函数式调用
 * @{
 */
void BEEP0_Set(uint8_t state);
uint8_t BEEP0_GetState(void);
/** @} */

/**
 * @defgroup KEY_Read_Functions 按键读取函数声明
 * @brief 提供按键读取的标准函数接口，支持函数式调用
 * @note 按键硬件设计为下拉模式，按下时为低电平(0)，释放时为高电平(1)
 * @{
 */
/**
 * @brief 按键0状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键0 (PA0)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY0_GetState(void);

/**
 * @brief 按键1状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键1 (PE2)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY1_GetState(void);

/**
 * @brief 按键2状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键2 (PE3)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY2_GetState(void);

/**
 * @brief 按键3状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键3 (PE4)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY3_GetState(void);
/** @} */

#endif
