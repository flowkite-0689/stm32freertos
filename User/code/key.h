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
// 按键按键常量定义
// ==================================

/**
 * @defgroup KEY_Press_Values 按键按键值定义
 * @{
 */
#define KEY0_PRES           1   ///< KEY0按键按下
#define KEY1_PRES           2   ///< KEY1按键按下
#define KEY2_PRES           4   ///< KEY2按键按下 (修改为位运算值)
#define KEY3_PRES           8   ///< KEY3按键按下 (修改为位运算值)
#define NUM_KEYS            4   ///< 按键数量
/** @} */

// ==================================
// 按键状态机结构体定义
// ==================================
/**
 * @brief 按键状态枚举
 */
typedef enum
{
    KEY_STATE_RELEASED = 0,      // 按键释放状态（已完成消抖确认）
    KEY_STATE_PRESSED = 1,      // 按键按下状态（已完成消抖确认）  
    KEY_STATE_PRESS_DEBOUNCE = 2, // 按键按下消抖中（等待稳定确认）
    KEY_STATE_RELEASE_DEBOUNCE = 3 // 按键释放消抖中（等待稳定确认）
} KeyState;

/**
 * @brief 按键状态机结构体
 * @note 用于非阻塞式按键消抖和事件管理
 */
typedef struct {
    KeyState current_state;      ///< 当前状态机状态
    uint8_t debounce_cnt;       ///< 消抖计数器
    uint8_t press_event;        ///< 按键事件标志 (按下后置1，读取后清0)
    uint8_t released_flag;      ///< 释放标志，用于单次模式
} Key_State_t;

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

/**
 * @brief 按键扫描函数（支持连续和非连续模式）
 * @param mode 扫描模式：0-支持连续按键，1-按键释放后才能再次检测
 * @return 按键按键值（KEY0_PRES~KEY3_PRES），0表示无按键按下
 * @note mode=0时，按下按键不松手也会连续返回按键值；mode=1时，必须松手才能再次检测
 */
uint8_t KEY_Scan(uint8_t mode);

// ==================================
// 非阻塞按键扫描函数声明
// ==================================

/**
 * @brief 非阻塞按键扫描函数（状态机消抖）
 * @note 此函数应定期调用（例如，每5ms调用一次），用于按键状态更新和消抖
 */
void KEY_Scan_NonBlocking(void);

/**
 * @brief 获取按键值（非阻塞，支持连续和非连续模式）
 * @param mode 扫描模式：0-支持连续按键，1-按键释放后才能再次检测（单次模式）
 * @return 按键按键值（KEY0_PRES~KEY3_PRES），0表示无按键按下
 */
uint8_t KEY_Get_Value(uint8_t mode);

// ==================================
// 按键中断函数声明
// ==================================

/**
 * @brief 初始化按键外部中断
 * @return 错误码：KEY_OK-成功，其他-失败
 * @note 配置所有按键为外部中断模式，下降沿触发
 */
int8_t KEY_EXTI_Init(void);

/**
 * @brief 按键消抖定时器初始化
 * @return 错误码：KEY_OK-成功，其他-失败
 * @note 使用TIM5作为20ms定时器进行消抖确认
 */
int8_t KEY_Debounce_Timer_Init(void);



// ==================================
// 按键中断相关全局变量声明
// ==================================

extern volatile uint8_t key_trig_flag;        // 按键触发标志

// ==================================
// 按键中断服务程序声明
// ==================================

void EXTI0_IRQHandler(void);  // KEY0 (PA0) 外部中断
void EXTI2_IRQHandler(void);  // KEY1 (PE2) 外部中断
void EXTI3_IRQHandler(void);  // KEY2 (PE3) 外部中断
void EXTI4_IRQHandler(void);  // KEY3 (PE4) 外部中断

/**
 * @brief 获取定时器消抖后的按键触发事件
 * @return 按键值：1,2,4,8 对应 KEY0~KEY3，无按键返回 0
 * @note 调用后会自动清除标志，线程安全
 */
static inline uint8_t KEY_Get(void)
{
    uint8_t trig = key_trig_flag;
    key_trig_flag = 0;          // 清除标志
    return trig;
}

#endif /* _KEY_H_ */
