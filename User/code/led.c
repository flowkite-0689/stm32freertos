/**
 * @file led.c
 * @brief STM32F4xx LED驱动模块实现
 * @author flowkite-0689
 * @version v2.0
 * @date 2025.11.10
 */

#include "led.h"

// ==================================
// LED初始化函数实现
// ==================================

/**
 * @brief LED GPIO初始化函数
 * @param LED_Pin LED引脚
 * @param LED_Port LED所在GPIO端口
 * @return 错误码：LED_OK-成功，其他-失败
 * @note 配置为高速输出模式，推挽输出，无上下拉
 */
int8_t LED_Init_Single(uint32_t LED_Pin, GPIO_TypeDef *LED_Port)
{
    return GPIO_Init_WithCheck(LED_Port, LED_Pin, GPIO_Mode_OUT, 
                              GPIO_High_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL);
}

/**
 * @brief 按编号初始化单个LED
 * @param led_num LED编号(0-3)
 * @return 错误码：LED_OK-成功，LED_PARAM_ERROR-参数错误，其他-失败
 */
int8_t LED_Init_ByNumber(uint8_t led_num)
{
    switch (led_num)
    {
        case 0:
            return LED_Init_Single(LED0_PIN, LED0_PORT);
        case 1:
            return LED_Init_Single(LED1_PIN, LED1_PORT);
        case 2:
            return LED_Init_Single(LED2_PIN, LED2_PORT);
        case 3:
            return LED_Init_Single(LED3_PIN, LED3_PORT);
        default:
            return LED_INVALID_LED;
    }
}

/**
 * @brief 初始化所有LED
 * @return 错误码：LED_OK-成功，其他-失败
 */
int8_t LED_Init(void)
{
    int8_t result;
    
    result = LED_Init_ByNumber(0);
    if (result != LED_OK) return result;
    
    result = LED_Init_ByNumber(1);
    if (result != LED_OK) return result;
    
    result = LED_Init_ByNumber(2);
    if (result != LED_OK) return result;
    
    result = LED_Init_ByNumber(3);
    if (result != LED_OK) return result;
    
    return LED_OK;
}

// ==================================
// LED控制函数实现
// ==================================

/**
 * @brief 设置LED状态
 * @param led_num LED编号(0-3)
 * @param state LED状态：0-亮，1-灭
 * @return 错误码：LED_OK-成功，LED_INVALID_LED-无效LED编号
 * @note 使用位带操作控制LED，硬件设计：低电平点亮，高电平熄灭
 */
int8_t LED_Set(uint8_t led_num, uint8_t state)
{
    switch (led_num)
    {
        case 0:
            LED0 = state;
            break;
        case 1:
            LED1 = state;
            break;
        case 2:
            LED2 = state;
            break;
        case 3:
            LED3 = state;
            break;
        default:
            return LED_INVALID_LED;
    }
    
    return LED_OK;
}

/**
 * @brief 切换LED状态
 * @param led_num LED编号(0-3)
 * @return 错误码：LED_OK-成功，LED_INVALID_LED-无效LED编号
 * @note 如果LED当前是亮的，则熄灭；如果是灭的，则点亮
 */
int8_t LED_Toggle(uint8_t led_num)
{
    uint8_t current_state;
    int8_t result = LED_Get_State(led_num, &current_state);
    
    if (result != LED_OK) {
        return result;
    }
    
    return LED_Set(led_num, current_state ? 0 : 1);  // 切换状态
}

/**
 * @brief 设置所有LED状态
 * @param state LED状态：0-全亮，1-全灭
 * @return 错误码：LED_OK-成功
 */
int8_t LED_Set_All(uint8_t state)
{
    LED0 = state;
    LED1 = state;
    LED2 = state;
    LED3 = state;
    
    return LED_OK;
}

// ==================================
// LED状态读取函数实现
// ==================================

/**
 * @brief 读取LED状态
 * @param led_num LED编号(0-3)
 * @param state 输出参数，指向存储LED状态的指针
 * @return 错误码：LED_OK-成功，LED_INVALID_LED-无效LED编号，LED_PARAM_ERROR-参数错误
 * @note 使用位带操作读取LED当前状态
 */
int8_t LED_Get_State(uint8_t led_num, uint8_t *state)
{
    if (state == NULL) {
        return LED_PARAM_ERROR;
    }
    
    switch (led_num)
    {
        case 0:
            *state = LED0_STATE();
            break;
        case 1:
            *state = LED1_STATE();
            break;
        case 2:
            *state = LED2_STATE();
            break;
        case 3:
            *state = LED3_STATE();
            break;
        default:
            return LED_INVALID_LED;
    }
    
    return LED_OK;
}

/**
 * @brief 检查LED是否点亮
 * @param led_num LED编号(0-3)
 * @return 1-点亮，0-熄灭，参数错误返回0
 */
uint8_t LED_Is_On(uint8_t led_num)
{
    uint8_t state;
    
    if (LED_Get_State(led_num, &state) != LED_OK) {
        return 0;
    }
    
    return !state;  // 低电平点亮，所以状态为0时是点亮状态
}

/**
 * @brief 检查LED是否熄灭
 * @param led_num LED编号(0-3)
 * @return 1-熄灭，0-点亮，参数错误返回0
 */
uint8_t LED_Is_Off(uint8_t led_num)
{
    uint8_t state;
    
    if (LED_Get_State(led_num, &state) != LED_OK) {
        return 0;
    }
    
    return state;  // 高电平熄灭，所以状态为1时是熄灭状态
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//-----------------------------------自己写的奇怪玩意--------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief 可中断的LED流水灯效果函数
 */
void ls_interruptible(void)
{
	uint32_t led_state[4] = {0, 0, 0, 0};
	uint32_t flag = 0;

	for (uint32_t i = 0; i < 4;)
	{
		// 检查是否有按键中断，如果有则退出
		if (KEY_Get() != 0)
		{
			return;
		}

		for (uint32_t j = 0; j < 4; j++)
		{
			led_state[j] = 1;
			if (j == i)
			{
				led_state[j] = 0;
			}
		}

		if (flag)
		{
			LED0 = led_state[0];
			LED1 = led_state[1];
			LED2 = led_state[2];
			LED3 = led_state[3];
		}
		else
		{
			LED3 = led_state[0];
			LED2 = led_state[1];
			LED1 = led_state[2];
			LED0 = led_state[3];
		}

		i++;
		if (i == 4)
		{
			flag = !flag;
		}

		i = i % 4;
		delay_ms(130);
	}
}
void ysysled()
{
	uint32_t last_LED0 = 0;
	uint32_t last_LED1 = 0;
	while (1)
	{
		if (KEY_Get() != 0)
		{
			return;
		}
		if ((get_systick() - last_LED0) >= 500)
		{
			LED0 = !LED0;
			last_LED0 = get_systick();
		}

		if ((get_systick() - last_LED1) >= 800)
		{
			LED1 = !LED1;
			last_LED1 = get_systick();
		}
	}
}
