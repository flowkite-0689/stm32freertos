#include "stm32f4xx.h"
#include "led.h"
#include "key.h"
#include "beep.h"

/**
 * @brief 主函数
 */
int main(void)
{
    uint8_t key_num;
    static uint8_t led_state[4] = {0, 0, 0, 0}; // 每个LED的状态

    // 初始化硬件
    KEY_Init();
    LED_Init();
    //    BEEP_Init();

    //    // 初始化LED为熄灭状态（高电平）
    LED_Set_All(0);

    //		LED0=0;
    while (1)
    {
        // 检测按键
        key_num = KEY_Read();

        // 如果有按键按下，切换对应LED的状态
        if (key_num < 4)
        {
            led_state[key_num] = !led_state[key_num];
            LED_Set(key_num, led_state[key_num]);

            // 蜂鸣器短鸣提示按键已响应
            //            BEEP_Buzz(0, 50);  // 蜂鸣器响50ms
        }

        // 小延时，降低CPU占用率
        Delay_ms(10);
    }
}
