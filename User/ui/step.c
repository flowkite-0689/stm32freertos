#include "step.h"
#include "ui/alarm_all.h"
#include "oled.h"
#include "oled_print.h"
#include "MPU6050.h"
#include "MPU6050/eMPL/inv_mpu_dmp_motion_driver.h"
#include "key.h"
#include "simple_pedometer.h"
#include "code/spi.h"
#include <stdint.h>

// 引用全局函数
extern uint32_t get_systick(void);
extern unsigned long g_step_count;




void step(void)
{
    delay_ms(10);
    OLED_Clear();
    OLED_Printf_Line(0, "Step Counter");
    OLED_Printf_Line(1, "KEY0-Reset KEY2-Back");
    OLED_Printf_Line(2, "Steps: %lu", g_step_count);
    OLED_Printf_Line(3, "Time: 0s");
    OLED_Refresh();
    
    // 不再初始化计步器，保持使用全局计数值
    
    unsigned long last_count = g_step_count;
    
    while(1)
    {
        // 全局闹钟处理 - 在计步器界面也能处理闹钟
        if (Alarm_GlobalHandler()) {
            continue; // 如果正在处理闹钟提醒，跳过计步器循环的其他部分
        }
        
        // 读取加速度数据
        short ax, ay, az;
        MPU_Get_Accelerometer(&ax, &ay, &az);
        
        // 使用简单计步器更新步数
        unsigned long count = simple_pedometer_update(ax, ay, az);
        
        OLED_Printf_Line(2, "Steps: %lu", count);
        OLED_Printf_Line(3, "Simple mode");
        OLED_Refresh_Dirty();
        
        // 检查步数变化
        if(count != last_count)
        {
            printf("!!! STEP DETECTED: %ld -> %ld !!!\r\n", last_count, count);
            last_count = count;
        }
        
        // 检查按键
        u8 key = KEY_Get();
        if(key != 0) 
        {
            switch(key)
            {
                case KEY0_PRES: // 短按KEY0重置步数
                    simple_pedometer_reset();
                    OLED_Printf_Line(2, "step reset!");
                    OLED_Printf_Line(3, "time reset!");
                    OLED_Refresh();
                    delay_ms(1000);
                    OLED_Printf_Line(2, "Steps: %lu", g_step_count);
                    OLED_Printf_Line(3, "Simple mode");
                    OLED_Refresh();
                    last_count = g_step_count;
                    break;
                    
                case KEY2_PRES: // 按KEY2返回菜单页面
                    OLED_Clear();
                    return;
                    
                default:
                    break;
            }
        }
        
        delay_ms(50); // 每50ms更新一次显示，提高响应速度
    }
}
