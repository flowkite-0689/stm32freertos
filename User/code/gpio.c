/**
 * @file gpio.c
 * @brief STM32F4xx GPIO底层操作模块实现
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 */

#include "gpio.h"

// ==================================
// GPIO时钟使能函数实现
// ==================================

/**
 * @brief 使能指定GPIO端口的时钟
 * @param GPIOx GPIO端口指针（如GPIOA、GPIOB等）
 * @return 错误码：GPIO_OK-成功，GPIO_PORT_ERROR-无效端口
 */
int8_t GPIO_Clock_Enable(GPIO_TypeDef *GPIOx)
{
    // 参数验证
    if (GPIOx == NULL) {
        return GPIO_PORT_ERROR;
    }
    
    // 根据GPIO端口使能对应时钟
    if (GPIOx == GPIOA) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    } else if (GPIOx == GPIOB) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    } else if (GPIOx == GPIOC) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    } else if (GPIOx == GPIOD) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    } else if (GPIOx == GPIOE) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    } else if (GPIOx == GPIOF) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    } else if (GPIOx == GPIOG) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    } else if (GPIOx == GPIOH) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    } else if (GPIOx == GPIOI) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
    } else {
        return GPIO_PORT_ERROR;  // 无效GPIO端口
    }
    
    return GPIO_OK;
}

// ==================================
// GPIO初始化函数实现
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
 */
int8_t GPIO_Init_WithCheck(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode,
                          GPIOSpeed_TypeDef GPIO_Speed, GPIOOType_TypeDef GPIO_OType,
                          GPIOPuPd_TypeDef GPIO_PuPd)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    int8_t clock_result;
    
    // 参数验证
    if (GPIOx == NULL) {
        return GPIO_PORT_ERROR;
    }
    
    if (GPIO_Pin == 0) {
        return GPIO_PARAM_ERROR;
    }
    
    // 使能GPIO时钟
    clock_result = GPIO_Clock_Enable(GPIOx);
    if (clock_result != GPIO_OK) {
        return clock_result;
    }
    
    // 配置GPIO参数
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode;
    GPIO_InitStructure.GPIO_OType = GPIO_OType;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd;
    
    // 初始化GPIO
    GPIO_Init(GPIOx, &GPIO_InitStructure);
    
    return GPIO_OK;
}

// ==================================
// GPIO操作函数实现
// ==================================

/**
 * @brief 设置GPIO引脚输出电平
 * @param GPIOx GPIO端口
 * @param GPIO_Pin 引脚位
 * @param state 输出状态：0-低电平，1-高电平
 * @return 错误码：GPIO_OK-成功，GPIO_PARAM_ERROR-参数错误，GPIO_PORT_ERROR-无效端口
 */
int8_t GPIO_Set_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t state)
{
    // 参数验证
    if (GPIOx == NULL) {
        return GPIO_PORT_ERROR;
    }
    
    if (GPIO_Pin == 0) {
        return GPIO_PARAM_ERROR;
    }
    
    // 设置输出状态
    if (state) {
        GPIO_SetBits(GPIOx, GPIO_Pin);  // 高电平
    } else {
        GPIO_ResetBits(GPIOx, GPIO_Pin);  // 低电平
    }
    
    return GPIO_OK;
}

/**
 * @brief 读取GPIO引脚输入电平
 * @param GPIOx GPIO端口
 * @param GPIO_Pin 引脚位
 * @param state 输出参数，指向存储读取状态的指针
 * @return 错误码：GPIO_OK-成功，GPIO_PARAM_ERROR-参数错误，GPIO_PORT_ERROR-无效端口
 */
int8_t GPIO_Read_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t *state)
{
    // 参数验证
    if (GPIOx == NULL || state == NULL) {
        return GPIO_PORT_ERROR;
    }
    
    if (GPIO_Pin == 0) {
        return GPIO_PARAM_ERROR;
    }
    
    // 读取输入状态
    *state = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
    
    return GPIO_OK;
}
