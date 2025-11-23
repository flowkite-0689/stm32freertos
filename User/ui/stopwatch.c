#include "stopwatch.h"
#include "ui/alarm_all.h"
#include <stdio.h>

/**
 * @brief 秒表界面
 */
void stopwatch(void)
{
    u8 key;
    static StopwatchState stopwatch_state = {0};  // 静态变量保存状态
    uint32_t last_update_time = 0;
    
    OLED_Clear();
    Display_Stopwatch(&stopwatch_state);
    
    while (1) {
        // 全局闹钟处理 - 在秒表界面也能处理闹钟
        if (Alarm_GlobalHandler()) {
            continue; // 如果正在处理闹钟提醒，跳过秒表循环的其他部分
        }
        
        if (key = KEY_Get()) {
            printf("Key pressed: %d\n", key);  // 调试信息
            switch (key) {
                case KEY0_PRES:  // 启动/继续
                    if (!stopwatch_state.running) {
                        stopwatch_state.running = 1;
                        stopwatch_state.start_time = get_systick();
                        printf("Stopwatch started\n");
                    }
                    break;
                    
                case KEY1_PRES:  // 暂停
                    if (stopwatch_state.running) {
                        stopwatch_state.running = 0;
                        stopwatch_state.pause_time += (get_systick() - stopwatch_state.start_time);
                        printf("Stopwatch paused\n");
                    }
                    break;
                    
                case KEY2_PRES:  // 退出
                    printf("Exiting stopwatch\n");
                    return;
                    
                case KEY3_PRES:  // 重置
                    stopwatch_state.running = 0;
                    stopwatch_state.start_time = 0;
                    stopwatch_state.pause_time = 0;
                    stopwatch_state.elapsed_time = 0;
                    printf("Stopwatch reset\n");
                    break;
            }
        }
        
        // 定时更新显示
        uint32_t current_time = get_systick();
        if (current_time - last_update_time >= STOPWATCH_UPDATE_INTERVAL) {
            last_update_time = current_time;
            Display_Stopwatch(&stopwatch_state);
        }
        
        // 如果正在运行，计算实时时间
        if (stopwatch_state.running) {
            stopwatch_state.elapsed_time = (current_time - stopwatch_state.start_time) + stopwatch_state.pause_time;
        }
    }
}

/**
 * @brief 显示秒表界面
 */
void Display_Stopwatch(StopwatchState* state)
{
    
    
    uint32_t total_seconds = state->elapsed_time / 1000;
    uint32_t minutes = (total_seconds / 60) % 60;
    uint32_t seconds = total_seconds % 60;
    uint32_t milliseconds = state->elapsed_time % 1000 / 10;
    
    OLED_Printf_Line_32(0, " %02lu:%02lu:%02lu", minutes, seconds, milliseconds);
    
    if(state->running) {
        OLED_Printf_Line(2, "    RUNNING");
        OLED_Printf_Line(3, "KEY1:Pause KEY2:Exit");
    } else {
        OLED_Printf_Line(2, "    PAUSED");
        OLED_Printf_Line(3, "KEY0:Start KEY3:Reset");
    }
    
    // 刷新OLED显示
    OLED_Refresh_Dirty();
}

