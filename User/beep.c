/**
 * @file beep.c
 * @brief STM32F4xx 蜂鸣器驱动模块实现
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 */

#include "beep.h"

// ==================================
// 蜂鸣器初始化函数实现
// ==================================

/**
 * @brief 蜂鸣器GPIO初始化函数
 * @param BEEP_Pin 蜂鸣器引脚
 * @param BEEP_Port 蜂鸣器所在GPIO端口
 * @return 错误码：BEEP_OK-成功，其他-失败
 * @note 配置为50MHz输出模式，推挽输出，无上下拉
 */
int8_t BEEP_Init_Single(uint32_t BEEP_Pin, GPIO_TypeDef *BEEP_Port)
{
    return GPIO_Init_WithCheck(BEEP_Port, BEEP_Pin, GPIO_Mode_OUT, 
                              GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL);
}

/**
 * @brief 按编号初始化单个蜂鸣器
 * @param beep_num 蜂鸣器编号
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号，其他-失败
 */
int8_t BEEP_Init_ByNumber(uint8_t beep_num)
{
    if (beep_num == 0) {
        return BEEP_Init_Single(BEEP0_PIN, BEEP0_PORT);
    }
    
    return BEEP_INVALID_BEEP;
}

/**
 * @brief 初始化所有蜂鸣器
 * @return 错误码：BEEP_OK-成功，其他-失败
 */
int8_t BEEP_Init(void)
{
    return BEEP_Init_ByNumber(0);
}

// ==================================
// 蜂鸣器控制函数实现
// ==================================

/**
 * @brief 设置蜂鸣器状态
 * @param beep_num 蜂鸣器编号
 * @param state 蜂鸣器状态：1-开启，0-关闭
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号
 * @note 使用位带操作控制蜂鸣器
 */
int8_t BEEP_Set(uint8_t beep_num, uint8_t state)
{
    if (beep_num == 0) {
        BEEP0(state);
        return BEEP_OK;
    }
    
    return BEEP_INVALID_BEEP;
}

/**
 * @brief 切换蜂鸣器状态
 * @param beep_num 蜂鸣器编号
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号
 * @note 如果蜂鸣器当前是开启的，则关闭；如果是关闭的，则开启
 */
int8_t BEEP_Toggle(uint8_t beep_num)
{
    uint8_t current_state;
    int8_t result = BEEP_Get_State(beep_num, &current_state);
    
    if (result != BEEP_OK) {
        return result;
    }
    
    return BEEP_Set(beep_num, current_state ? 0 : 1);  // 切换状态
}

/**
 * @brief 蜂鸣器发声
 * @param beep_num 蜂鸣器编号
 * @param duration_ms 持续时间（毫秒）
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号，BEEP_PARAM_ERROR-参数错误
 * @note 控制蜂鸣器发声指定时间后自动停止
 */
int8_t BEEP_Buzz(uint8_t beep_num, uint32_t duration_ms)
{
    if (duration_ms == 0) {
        return BEEP_PARAM_ERROR;
    }
    
    // 开启蜂鸣器
    int8_t result = BEEP_Set(beep_num, 1);
    if (result != BEEP_OK) {
        return result;
    }
    
    // 延时指定时间
    delay_ms(duration_ms);
    
    // 关闭蜂鸣器
    return BEEP_Set(beep_num, 0);
}

// ==================================
// 蜂鸣器状态读取函数实现
// ==================================

/**
 * @brief 读取蜂鸣器状态
 * @param beep_num 蜂鸣器编号
 * @param state 输出参数，指向存储蜂鸣器状态的指针
 * @return 错误码：BEEP_OK-成功，BEEP_INVALID_BEEP-无效蜂鸣器编号，BEEP_PARAM_ERROR-参数错误
 * @note 使用位带操作读取蜂鸣器当前状态
 */
int8_t BEEP_Get_State(uint8_t beep_num, uint8_t *state)
{
    if (state == NULL) {
        return BEEP_PARAM_ERROR;
    }
    
    if (beep_num == 0) {
        *state = BEEP0_STATE();
        return BEEP_OK;
    }
    
    return BEEP_INVALID_BEEP;
}

/**
 * @brief 检查蜂鸣器是否开启
 * @param beep_num 蜂鸣器编号
 * @return 1-开启，0-关闭，参数错误返回0
 */
uint8_t BEEP_Is_On(uint8_t beep_num)
{
    uint8_t state;
    
    if (BEEP_Get_State(beep_num, &state) != BEEP_OK) {
        return 0;
    }
    
    return state;  // 高电平开启
}

/**
 * @brief 检查蜂鸣器是否关闭
 * @param beep_num 蜂鸣器编号
 * @return 1-关闭，0-开启，参数错误返回0
 */
uint8_t BEEP_Is_Off(uint8_t beep_num)
{
    uint8_t state;
    
    if (BEEP_Get_State(beep_num, &state) != BEEP_OK) {
        return 0;
    }
    
    return !state;  // 低电平关闭
}
