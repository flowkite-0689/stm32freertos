/**
 * @file key.c
 * @brief STM32F4xx 按键驱动模块实现
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 */

#include "key.h"

// ==================================
// 按键初始化函数实现
// ==================================

/**
 * @brief 按键GPIO初始化函数
 * @param KEY_Pin 按键引脚
 * @param KEY_Port 按键所在GPIO端口
 * @return 错误码：KEY_OK-成功，其他-失败
 * @note 配置为高速输入模式，推挽输出，无上下拉
 */
int8_t KEY_Init_Single(uint32_t KEY_Pin, GPIO_TypeDef *KEY_Port)
{
    return GPIO_Init_WithCheck(KEY_Port, KEY_Pin, GPIO_Mode_IN, 
                               GPIO_High_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL);
}

/**
 * @brief 按编号初始化单个按键
 * @param key_num 按键编号(0-3)
 * @return 错误码：KEY_OK-成功，KEY_INVALID_KEY-无效按键编号，其他-失败
 */
int8_t KEY_Init_ByNumber(uint8_t key_num)
{
    switch (key_num)
    {
        case 0:
            return KEY_Init_Single(KEY0_PIN, KEY0_PORT);
        case 1:
            return KEY_Init_Single(KEY1_PIN, KEY1_PORT);
        case 2:
            return KEY_Init_Single(KEY2_PIN, KEY2_PORT);
        case 3:
            return KEY_Init_Single(KEY3_PIN, KEY3_PORT);
        default:
            return KEY_INVALID_KEY;
    }
}

/**
 * @brief 初始化所有按键
 * @return 错误码：KEY_OK-成功，其他-失败
 */
int8_t KEY_Init(void)
{
    int8_t result;
    
    result = KEY_Init_ByNumber(0);
    if (result != KEY_OK) return result;
    
    result = KEY_Init_ByNumber(1);
    if (result != KEY_OK) return result;
    
    result = KEY_Init_ByNumber(2);
    if (result != KEY_OK) return result;
    
    result = KEY_Init_ByNumber(3);
    if (result != KEY_OK) return result;
    
    return KEY_OK;
}

// ==================================
// 按键状态读取函数实现
// ==================================

/**
 * @brief 读取单个按键状态
 * @param key_num 按键编号(0-3)
 * @param state 输出参数，指向存储按键状态的指针
 * @return 错误码：KEY_OK-成功，KEY_INVALID_KEY-无效按键编号，KEY_PARAM_ERROR-参数错误
 * @note 按键硬件设计为下拉模式，按下时为低电平(0)，释放时为高电平(1)
 */
int8_t KEY_Get_State(uint8_t key_num, uint8_t *state)
{
    if (state == NULL) {
        return KEY_PARAM_ERROR;
    }
    
    switch (key_num)
    {
        case 0:
            *state = KEY0;
            break;
        case 1:
            *state = KEY1;
            break;
        case 2:
            *state = KEY2;
            break;
        case 3:
            *state = KEY3;
            break;
        default:
            return KEY_INVALID_KEY;
    }
    
    return KEY_OK;
}

/**
 * @brief 检查按键是否按下
 * @param key_num 按键编号(0-3)
 * @return 1-按下，0-释放，参数错误返回0
 */
uint8_t KEY_Is_Pressed(uint8_t key_num)
{
    uint8_t state;
    
    if (KEY_Get_State(key_num, &state) != KEY_OK) {
        return 0;
    }
    
    return !state;  // 下拉模式，按下时为低电平
}

/**
 * @brief 检查按键是否释放
 * @param key_num 按键编号(0-3)
 * @return 1-释放，0-按下，参数错误返回0
 */
uint8_t KEY_Is_Released(uint8_t key_num)
{
    uint8_t state;
    
    if (KEY_Get_State(key_num, &state) != KEY_OK) {
        return 0;
    }
    
    return state;  // 下拉模式，释放时为高电平
}

// ==================================
// 按键消抖函数实现
// ==================================

/**
 * @brief 按键消抖检测
 * @param key_num 按键编号(0-3)
 * @return 错误码：KEY_OK-确认按下，其他-未按下或抖动/参数错误
 * @note 进行延时消抖确认
 */
int8_t KEY_Debounce(uint8_t key_num)
{
    // 延时消抖，确认按键按下
    Delay_ms(15);
    
    // 再次检测按键状态
    return KEY_Is_Pressed(key_num) ? KEY_OK : KEY_NONE_PRESSED;
}

/**
 * @brief 等待按键释放
 * @param key_num 按键编号(0-3)
 * @return 错误码：KEY_OK-成功，其他-参数错误
 * @note 阻塞等待按键释放
 */
int8_t KEY_WaitForRelease(uint8_t key_num)
{
    if (key_num > 3) {
        return KEY_INVALID_KEY;
    }
    
    while (KEY_Is_Pressed(key_num))  // 等待按键释放
    {
        Delay_ms(10);  // 小延时，降低CPU占用率
    }
    
    return KEY_OK;
}

// ==================================
// 按键检测函数实现
// ==================================

/**
 * @brief 获取按键状态（带消抖和等待释放）
 * @return 按键编号(0~3)，KEY_NONE_PRESSED表示无按键按下
 * @note 采用"快速检测+延时确认+等待释放"的处理逻辑
 */
uint8_t KEY_Read(void)
{
    uint8_t key_pressed = KEY_NONE_PRESSED;  // 初始化为无按键

    // 1. 快速检测四个按键的输入引脚
    for (uint8_t i = 0; i < 4; i++)
    {
        if (KEY_Is_Pressed(i))  // 直接使用封装好的函数
        {
            key_pressed = i;  // 记录可能按下的按键
            break;            // 退出循环，不再检测其他按键
        }
    }

    // 2. 如果没有按键按下，直接返回
    if (key_pressed == KEY_NONE_PRESSED)
    {
        return KEY_NONE_PRESSED;
    }

    // 3. 消抖检测
    if (KEY_Debounce(key_pressed) == KEY_OK)
    {
        // 4. 等待按键释放
        KEY_WaitForRelease(key_pressed);
        
        // 5. 确认按键"按下-释放"过程后，返回按键编号
        return key_pressed;
    }

    // 如果是干扰抖动，说明没有有效按键
    return KEY_NONE_PRESSED;
}
