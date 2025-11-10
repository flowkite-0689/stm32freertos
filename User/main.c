#include "stm32f4xx.h"
#include "./myInit/myInit.h"

/**
 * @brief  获取按键状态（改进的健壮实现）
 * @param  None
 * @retval uint8_t: 0~3分别对应KEY0~KEY3按下，0xFF表示无按键按下
 * @note   采用"快速检测+延时确认+等待释放"三步逻辑
 */
uint8_t Key_Read(void)
{
    uint8_t key_pressed = 0xFF; // 初始化为无按键
    uint8_t current_state;

    // 1. 快速检测四个按键的按下情况
    for(uint8_t i = 0; i < 4; i++)
    {
        switch(i)
        {
            case 0: current_state = GPIO_ReadInputDataBit(KEY0_PORT, KEY0_PIN); break;
            case 1: current_state = GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN); break;
            case 2: current_state = GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN); break;
            case 3: current_state = GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN); break;
            default: current_state = 1; break; // 默认为释放
        }

        // 2. 检测到按键按下（低电平）
        if(current_state == 0)
        {
            key_pressed = i; // 记录可能按下的按键
            break; // 跳出循环，不再检测其他按键
        }
    }

    // 3. 如果没有按键按下，直接返回0xFF
    if(key_pressed == 0xFF)
    {
        return 0xFF;
    }

    // 4. 延时消抖，确认按键按下（防止抖动）
    Delay_ms(15);

    // 5. 再次确认按键是否仍然按下
    switch(key_pressed)
    {
        case 0: current_state = GPIO_ReadInputDataBit(KEY0_PORT, KEY0_PIN); break;
        case 1: current_state = GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN); break;
        case 2: current_state = GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN); break;
        case 3: current_state = GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN); break;
        default: current_state = 1; break;
    }

    // 6. 确认按键
    if(current_state == 0)
    {
        // 7. 等待按键释放
        while(1)
        {
            switch(key_pressed)
            {
                case 0: current_state = GPIO_ReadInputDataBit(KEY0_PORT, KEY0_PIN); break;
                case 1: current_state = GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN); break;
                case 2: current_state = GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN); break;
                case 3: current_state = GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN); break;
            }
            if(current_state == 1) // 检测到按键释放
            {
                break; // 退出等待循环
            }
            Delay_ms(10); // 小延时降低CPU占用率
        }
        
        // 8. 确认完整的"按下-释放"过程后，返回按键编号
        return key_pressed;
    }

    // 如果是抖动或干扰，说明没有有效按键
    return 0xFF;
}

/**
 * @brief 控制LED开关
 * @param led_num LED编号(0-3)
 * @param state LED状态(0-开启, 1-关闭)
 */
void LED_Control(uint8_t led_num, uint8_t state) {
    switch(led_num) {
        case 0: // LED0
            if (state) {
                GPIO_SetBits(LED0_PORT, LED0_PIN);      // 关闭LED
            } else {
                GPIO_ResetBits(LED0_PORT, LED0_PIN);    // 点亮LED
            }
            break;
        case 1: // LED1
            if (state) {
                GPIO_SetBits(LED1_PORT, LED1_PIN);
            } else {
                GPIO_ResetBits(LED1_PORT, LED1_PIN);
            }
            break;
        case 2: // LED2
            if (state) {
                GPIO_SetBits(LED2_PORT, LED2_PIN);
            } else {
                GPIO_ResetBits(LED2_PORT, LED2_PIN);
            }
            break;
        case 3: // LED3
            if (state) {
                GPIO_SetBits(LED3_PORT, LED3_PIN);
            } else {
                GPIO_ResetBits(LED3_PORT, LED3_PIN);
            }
            break;
    }
}

/**
 * @brief 主函数
 */
int main(void) {
    uint8_t key_num;
    static uint8_t led_state[4] = {0, 0, 0, 0}; // 每个LED的状态
    
    // 初始化硬件
    // KEY_Init(KEY0_PIN, KEY0_PORT);
    // KEY_Init(KEY1_PIN, KEY1_PORT);
    // KEY_Init(KEY2_PIN, KEY2_PORT);
    // KEY_Init(KEY3_PIN, KEY3_PORT);

    KEY_Initx(0);
    KEY_Initx(1);
    KEY_Initx(2);
    KEY_Initx(3);

    
//    LED_Init(LED0_PIN, LED0_PORT);
//    LED_Init(LED1_PIN, LED1_PORT);
//    LED_Init(LED2_PIN, LED2_PORT);
//    LED_Init(LED3_PIN, LED3_PORT);
		
		LED_Initx(0);
		LED_Initx(1);
		LED_Initx(2);
		LED_Initx(3);
		
    
    // 初始化LED为开启状态
    GPIO_ResetBits(LED0_PORT, LED0_PIN);
    GPIO_ResetBits(LED1_PORT, LED1_PIN);
    GPIO_ResetBits(LED2_PORT, LED2_PIN);
    GPIO_ResetBits(LED3_PORT, LED3_PIN);
    
    while(1) {
        // 检测按键
        key_num = Key_Read();
        
        // 如果有按键按下，切换对应LED的状态
        if (key_num < 4) {
            led_state[key_num] = !led_state[key_num];
            LED_Control(key_num, led_state[key_num]);
        }
        
        // 短暂延时降低CPU占用率
       Delay_ms(10);
    }
}
