/**
 * @file key.c
 * @brief STM32F4xx 按键驱动模块实现
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 */

#include "key.h"

// ==================================
// 按键状态机全局变量
// ==================================

Key_State_t keys[NUM_KEYS]; // 全局变量，存储所有按键状态

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
    
    // 初始化所有按键状态为默认值
    for (uint8_t i = 0; i < NUM_KEYS; i++)
    {
        keys[i].current_state = 1;     // 假设初始状态为释放（高电平）
        keys[i].debounce_cnt = 0;      // 消抖计数器清零
        keys[i].press_event = 0;       // 按键事件标志清零
        keys[i].released_flag = 1;     // 释放标志初始化为已释放
    }
    
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
    delay_ms(15);
    
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
        delay_ms(10);  // 小延时，降低CPU占用率
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

// ==================================
// 按键扫描函数实现
// ==================================

/**
 * @brief 按键扫描函数（支持连续和非连续模式）
 * @param mode 扫描模式：0-支持连续按键，1-按键释放后才能再次检测
 * @return 按键按键值（KEY0_PRES~KEY3_PRES），0表示无按键按下
 * @note mode=0时，按下按键不松手也会连续返回按键值；mode=1时，必须松手才能再次检测
 */
uint8_t KEY_Scan(uint8_t mode)
{
    // 调用非阻塞扫描函数更新按键状态
    KEY_Scan_NonBlocking();
    
    // 使用改进的按键值获取函数
    return KEY_Get_Value(mode);
}

// ==================================
// 非阻塞按键扫描函数实现
// ==================================

/**
 * @brief 读取按键引脚电平的宏实现
 * @param key_num 按键编号(0-3)
 * @return 当前引脚电平（0-低电平，1-高电平）
 */
#define KEY_READ(key_num) ( \
    (key_num == 0) ? KEY0 : \
    (key_num == 1) ? KEY1 : \
    (key_num == 2) ? KEY2 : \
    (key_num == 3) ? KEY3 : 1)

/**
 * @brief 非阻塞按键扫描函数（状态机消抖）
 * @note 此函数应定期调用（例如，每5ms调用一次），用于按键状态更新和消抖
 */
void KEY_Scan_NonBlocking(void)
{
    for (uint8_t i = 0; i < NUM_KEYS; i++)
    {
        // 1. 读取当前原始电平 (假设低电平有效，按下时为0)
        uint8_t current_pin_level = KEY_READ(i); 

        // 2. 状态机消抖逻辑
        if (current_pin_level != keys[i].current_state)
        {
            // 电平发生变化，开始消抖计数
            if (keys[i].debounce_cnt < 10) // 计数到 10 次 (例如 10*5ms = 50ms)
            {
                keys[i].debounce_cnt++;
            }
            else
            {
                // 消抖完成，确认状态改变
                keys[i].current_state = current_pin_level;

                // 如果状态变为按下 (假设按下是低电平 0)
                if (keys[i].current_state == 0)
                {
                    keys[i].press_event = 1; // 标记按键按下事件
                }
            }
        }
        else
        {
            // 状态稳定，重置消抖计数器
            keys[i].debounce_cnt = 0;
        }
    }
}

/**
 * @brief 获取按键值（非阻塞，支持连续和非连续模式）
 * @param mode 扫描模式：0-支持连续按键，1-按键释放后才能再次检测（单次模式）
 * @return 按键按键值（KEY0_PRES~KEY3_PRES），0表示无按键按下
 */
uint8_t KEY_Get_Value(uint8_t mode)
{
    for (uint8_t i = 0; i < NUM_KEYS; i++)
    {
        // --- 模式 1: 单次触发模式 (必须释放才能再次检测) ---
        if (mode == 1)
        {
            if (keys[i].current_state == 0) {
                // 按键当前是按下的，标记为未释放
                keys[i].released_flag = 0; 
            } else {
                // 按键当前是释放的，标记为已释放
                keys[i].released_flag = 1;
            }

            if (keys[i].press_event && keys[i].released_flag == 1)
            {
                 // 必须是新事件 且 之前处于释放状态
                 keys[i].press_event = 0; // 清除事件标志
                 // 返回对应的按键值（根据您的定义 KEY0_PRES 等）
                 return (KEY0_PRES << i); 
            }
        }
        // --- 模式 0: 连续触发模式 ---
        else 
        {
            if (keys[i].press_event)
            {
                keys[i].press_event = 0; // 清除事件标志
                return (KEY0_PRES << i);
            }
        }
    }
    
    return 0; // 无按键事件
}
