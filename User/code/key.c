/**
 * @file key.c
 * @brief STM32F4xx 按键驱动模块实现
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 */

#include "key.h"
#include "stm32f4xx_exti.h"
#include "misc.h"

// ==================================
// 按键状态机全局变量
// ==================================

Key_State_t keys[NUM_KEYS]; // 全局变量，存储所有按键状态

// ==================================
// 按键中断相关全局变量
// ==================================

volatile uint8_t key_interrupt_flag = 0;   // 按键中断标志
volatile uint8_t key_interrupt_value = 0;  // 按键中断值
volatile uint8_t key_trig_flag = 0;         // 哪个按键触发了（位标志：bit0~bit3）

// ==================================
// 定时器中断消抖全局变量
// ==================================

volatile uint8_t key_debounce_active = 0;    // 正在消抖中
volatile uint8_t key_pending_check = 0xFF;   // 哪个按键需要确认（bit0~3）

// ==================================
// 按键定时器消抖处理函数
// ==================================

/**
 * @brief 统一的按键外部中断触发消抖函数
 * @param key_num 按键编号(0-3)
 * @param exti_line 外部中断线
 * @note 立即关闭中断并启动定时器进行消抖确认
 */
static void KEY_EXTI_Trigger_Debounce(uint8_t key_num, uint32_t exti_line)
{
    if (EXTI_GetITStatus(exti_line) != RESET)
    {
        EXTI_ClearITPendingBit(exti_line);
        
        // 关键：立即关闭本线中断，防止抖动重复触发
        EXTI->IMR &= ~exti_line;
        
        // 标记哪个按键需要消抖确认
        key_pending_check = key_num;
        key_debounce_active = 1;
        
        // 启动定时器5，20ms 后确认
        TIM_SetCounter(TIM5, 0);
        TIM_Cmd(TIM5, ENABLE);
    }
}

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
        keys[i].current_state = KEY_STATE_RELEASED; // 正确初始化为释放状态
        keys[i].debounce_cnt = 0;                   // 消抖计数器清零
        keys[i].press_event = 0;                    // 按键事件标志清零
        keys[i].released_flag = 1;                  // 释放标志初始化为已释放
    }

    result = KEY_Init_ByNumber(0);
    if (result != KEY_OK)
        return result;

    result = KEY_Init_ByNumber(1);
    if (result != KEY_OK)
        return result;

    result = KEY_Init_ByNumber(2);
    if (result != KEY_OK)
        return result;

    result = KEY_Init_ByNumber(3);
    if (result != KEY_OK)
        return result;

    result = KEY_EXTI_Init(); // 加入按键外部中断初始化到初始化函数中
    if (result != KEY_OK)
        return result;

    result = KEY_Debounce_Timer_Init(); // 初始化定时器用于消抖
    if (result != KEY_OK)
        return result;

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
    if (state == NULL)
    {
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

    if (KEY_Get_State(key_num, &state) != KEY_OK)
    {
        return 0;
    }

    return !state; // 下拉模式，按下时为低电平
}

/**
 * @brief 检查按键是否释放
 * @param key_num 按键编号(0-3)
 * @return 1-释放，0-按下，参数错误返回0
 */
uint8_t KEY_Is_Released(uint8_t key_num)
{
    uint8_t state;

    if (KEY_Get_State(key_num, &state) != KEY_OK)
    {
        return 0;
    }

    return state; // 下拉模式，释放时为高电平
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
    if (key_num > 3)
    {
        return KEY_INVALID_KEY;
    }

    while (KEY_Is_Pressed(key_num)) // 等待按键释放
    {
        delay_ms(10); // 小延时，降低CPU占用率
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
    uint8_t key_pressed = KEY_NONE_PRESSED; // 初始化为无按键

    // 1. 快速检测四个按键的输入引脚
    for (uint8_t i = 0; i < 4; i++)
    {
        if (KEY_Is_Pressed(i)) // 直接使用封装好的函数
        {
            key_pressed = i; // 记录可能按下的按键
            break;           // 退出循环，不再检测其他按键
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
#define KEY_READ(key_num) (                       \
    (key_num == 0) ? KEY0 : (key_num == 1) ? KEY1 \
                        : (key_num == 2)   ? KEY2 \
                        : (key_num == 3)   ? KEY3 \
                                           : 1)

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
        switch (keys[i].current_state)
        {
        // 状态0：按键已释放（稳定状态）
        case KEY_STATE_RELEASED:
            // 若当前电平为低电平（检测到按键按下动作）
            if (current_pin_level == 0)
            {
                // 进入按下消抖状态，开始消抖计数
                keys[i].current_state = KEY_STATE_PRESS_DEBOUNCE;
                keys[i].debounce_cnt = 1;
            }
            break;

        // 状态1：按键按下消抖中（等待确认是否真的按下）
        case KEY_STATE_PRESS_DEBOUNCE:
            // 若当前仍为低电平（按下状态持续）
            if (current_pin_level == 0)
            {
                // 消抖计数器累加
                keys[i].debounce_cnt++;
                // 当连续3次检测到低电平，确认按键真的按下
                if (keys[i].debounce_cnt >= 3)
                {
                    // 切换到稳定按下状态
                    keys[i].current_state = KEY_STATE_PRESSED;
                    keys[i].press_event = 1; // 标记按键按下事件
                }
            }
            else
            {
                // 若中途检测到高电平，可能是抖动，退回释放状态
                keys[i].current_state = KEY_STATE_RELEASED;
                keys[i].debounce_cnt = 0;
            }
            break;

        // 状态2：按键已按下（稳定状态）
        case KEY_STATE_PRESSED:
            // 若当前电平为高电平（检测到按键释放动作）
            if (current_pin_level == 1)
            {
                // 进入释放消抖状态，开始消抖计数
                keys[i].current_state = KEY_STATE_RELEASE_DEBOUNCE;
                keys[i].debounce_cnt = 1;
            }
            break;

        // 状态3：按键释放消抖中（等待确认是否真的释放）
        case KEY_STATE_RELEASE_DEBOUNCE:
            // 若当前仍为高电平（释放状态持续）
            if (current_pin_level == 1)
            {
                // 消抖计数器累加
                keys[i].debounce_cnt++;
                // 当连续3次检测到高电平，确认按键真的释放
                if (keys[i].debounce_cnt >= 3)
                {
                    // 切换到稳定释放状态
                    keys[i].current_state = KEY_STATE_RELEASED;
                }
            }
            else
            {
                // 若中途检测到低电平，可能是抖动，退回按下状态
                keys[i].current_state = KEY_STATE_PRESSED;
                keys[i].debounce_cnt = 0;
            }
            break;

        default:
            // 异常状态，重置为释放状态
            keys[i].current_state = KEY_STATE_RELEASED;
            keys[i].debounce_cnt = 0;
            break;
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
            if (keys[i].current_state == KEY_STATE_PRESSED)
            {
                // 按键当前是按下的，标记为未释放
                keys[i].released_flag = 0;
            }
            else
            {
                // 按键当前是释放的，标记为已释放
                keys[i].released_flag = 1;
            }

            if (keys[i].press_event && keys[i].released_flag == 1)
            {
                // 必须是新事件 且 之前处于释放状态
                keys[i].press_event = 0; // 清除事件标志
                // 返回对应的按键值（根据您的定义 KEY0_PRES 等）
                return (1 << i);
            }
        }
        // --- 模式 0: 连续触发模式 ---
        else
        {
            if (keys[i].press_event)
            {
                keys[i].press_event = 0; // 清除事件标志
                return (1 << i);
            }
        }
    }

    return 0; // 无按键事件
}

// ==================================
// 按键中断函数实现
// ==================================

/**
 * @brief 初始化按键外部中断
 * @return 错误码：KEY_OK-成功，其他-失败
 * @note 配置所有按键为外部中断模式，下降沿触发
 */
int8_t KEY_EXTI_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // // 1. 初始化所有按键GPIO（如果尚未初始化）
    // int8_t result = KEY_Init();
    // if (result != KEY_OK) return result;//服了。。

    // 2. 使能SYSCFG时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 配置中断优先级组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 2位抢占优先级，2位子优先级

    // 3. 配置KEY0 (PA0)的外部中断
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 配置KEY0中断优先级 - 降低优先级，避免影响系统复位
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 4. 配置KEY1 (PE2)的外部中断
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);

    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_Init(&EXTI_InitStructure);

    // 配置KEY1中断优先级 

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
     
    NVIC_Init(&NVIC_InitStructure);

    // 5. 配置KEY2 (PE3)的外部中断
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);

    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_Init(&EXTI_InitStructure);

    // 配置KEY2中断优先级 - 降低优先级
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
     
    NVIC_Init(&NVIC_InitStructure);

    // 6. 配置KEY3 (PE4)的外部中断
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);

    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_Init(&EXTI_InitStructure);

    // 配置KEY3中断优先级 - 降低优先级

    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
     
    NVIC_Init(&NVIC_InitStructure);

    return KEY_OK;
}

/**
 * @brief 获取中断按键值
 * @return 按键按键值（KEY0_PRES~KEY3_PRES），0表示无按键事件
 * @note 非阻塞方式获取按键中断事件
 */

// ==================================
// 按键中断服务程序实现
// ==================================

/**
 * @brief EXTI Line0 中断服务程序 (KEY0 - PA0)
 */
void EXTI0_IRQHandler(void)
{
    KEY_EXTI_Trigger_Debounce(0, EXTI_Line0);
}

/**
 * @brief EXTI Line2 中断服务程序 (KEY1 - PE2)
 */
void EXTI2_IRQHandler(void)
{
    KEY_EXTI_Trigger_Debounce(1, EXTI_Line2);
}

/**
 * @brief EXTI Line3 中断服务程序 (KEY2 - PE3)
 */
void EXTI3_IRQHandler(void)
{
    KEY_EXTI_Trigger_Debounce(2, EXTI_Line3);
}

/**
 * @brief EXTI Line4 中断服务程序 (KEY3 - PE4)
 */
void EXTI4_IRQHandler(void)
{
    KEY_EXTI_Trigger_Debounce(3, EXTI_Line4);
}

/**
 * @brief 按键消抖定时器初始化
 * @return 错误码：KEY_OK-成功，其他-失败
 * @note 使用TIM5作为20ms定时器进行消抖确认
 */
int8_t KEY_Debounce_Timer_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM5时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    
    // 系统时钟 168MHz，APB1 分频后 84MHz
    TIM_TimeBaseStructure.TIM_Period = 25000 - 1;        // 25ms
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;        // 1MHz计数频率
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM5, DISABLE);  // 默认关闭，外部中断触发时再开
    
    // 配置TIM5中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    return KEY_OK;
}

// ==================================
// 定时器中断服务程序
// ==================================

/**
 * @brief TIM5定时器中断服务程序 - 按键消抖确认
 */
void TIM5_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
        TIM_Cmd(TIM5, DISABLE);
        
        if (key_debounce_active && key_pending_check < 4)
        {
            // 最终确认电平是否还是按下状态
            uint8_t current_level = 0;
            KEY_Get_State(key_pending_check, &current_level);
            
            if (current_level == 0)  // 仍然是低电平 → 确认有效按下
            {
                key_trig_flag |= (1 << key_pending_check);   // 打事件标志
                // 也可以直接放消息队列、事件标志组等
            }
            
            // 重新打开对应线的外部中断
            switch(key_pending_check)
            {
                case 0: EXTI->IMR |= EXTI_Line0; break;
                case 1: EXTI->IMR |= EXTI_Line2; break;
                case 2: EXTI->IMR |= EXTI_Line3; break;
                case 3: EXTI->IMR |= EXTI_Line4; break;
            }
        }
        
        key_debounce_active = 0;
        key_pending_check = 0xFF;
    }
}
