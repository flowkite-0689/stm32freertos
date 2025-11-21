#include "alarm_menu.h"
#include <stdio.h>

// 选项的图标
const unsigned char *alarm_menu_options[] =
    {
        gImage_add,      // 新建闹钟图标
        gImage_list      // 闹钟列表图标
};

void alarm_menu_Enter_select(u8 selected)
{
    switch (selected)
    {
    case 0:
        // 新建闹钟功能
        alarm_create();
        break;
    case 1:
        // 闹钟列表功能
        alarm_list();
        break;
    
    default:
        break;
    }
}

void alarm_menu_Ref(u8 selected)
{
    u8 right = ((selected + 1) % alarm_menu_options_NUM);

    OLED_ShowPicture(48, 16, 32, 32, alarm_menu_options[selected], 0);
    OLED_ShowPicture(96, 16, 32, 32, alarm_menu_options[right], 1);
    OLED_Refresh();
}

// 设置步骤和临时变量
static u8 set_alarm_step = 0;
static Alarm_TypeDef temp_alarm = {0};

/**
 * @brief 显示闹钟设置界面
 */
void Display_Set_Alarm(void)
{
    // 根据设置步骤高亮显示当前设置项
    switch(set_alarm_step) {
        case 0:  // 设置小时
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "  [%02d]:%02d:%02d-loop:%s", temp_alarm.hour,
                 temp_alarm.minute, temp_alarm.second,temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "    Set Hours");
            break;
        case 1:  // 设置分钟
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %02d:[%02d]:%02d-loop:%s", temp_alarm.hour, temp_alarm.minute, 
                temp_alarm.second,temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Minutes");
            break;
        case 2:  // 设置秒
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %02d:%02d:[%02d]-loop:%s", temp_alarm.hour, temp_alarm.minute, 
                temp_alarm.second,temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Seconds");
            break;
        case 3:  // 设置重复
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, "   %02d:%02d:%02d-loop:[%s]", temp_alarm.hour, 
                temp_alarm.minute, temp_alarm.second,temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Repeat");
            break;
    }
    
    // 只在初始化时显示标题和提示
   
        OLED_Printf_Line(0, "  SET ALARM");
        OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
     
    
    // 刷新OLED显示
    OLED_Refresh_Dirty();
}

/**
 * @brief 处理闹钟设置交互
 */
void Process_Set_Alarm(Alarm_TypeDef* alarm)
{
    u8 key;
    
    printf("Entering alarm setting mode\n");  // 调试信息
    
    // 初始化界面
    OLED_Clear();
    set_alarm_step = 0;
    
    // 如果提供了现有闹钟，则使用其值作为初始值
    if (alarm != NULL) {
        temp_alarm = *alarm;
    } else {
        temp_alarm.hour = 0;
        temp_alarm.minute = 0;
        temp_alarm.second = 0;
        temp_alarm.repeat = 0;
        temp_alarm.enabled = 1;
    }
    
    printf("Setting alarm: %02d:%02d:%02d\n", temp_alarm.hour, temp_alarm.minute, temp_alarm.second);  // 调试信息
    Display_Set_Alarm();
    
    while (1) {
        delay_ms(10);
        if ((key = KEY_Get())!=0) {
            printf("Key pressed: %d\n", key);  // 调试信息
            switch (key) {
                case KEY0_PRES:  // 增加
                    switch(set_alarm_step) {
                        case 0:  // 小时
                            temp_alarm.hour = (temp_alarm.hour + 1) % 24;
                            break;
                        case 1:  // 分钟
                            temp_alarm.minute = (temp_alarm.minute + 1) % 60;
                            break;
                        case 2:  // 秒
                            temp_alarm.second = (temp_alarm.second + 1) % 60;
                            break;
                        case 3:  // 重复
                            temp_alarm.repeat = !temp_alarm.repeat;
                            break;
                    }
                    Display_Set_Alarm();
                    break;
                    
                case KEY1_PRES:  // 减少
                    switch(set_alarm_step) {
                        case 0:  // 小时
                            temp_alarm.hour = (temp_alarm.hour == 0) ? 23 : temp_alarm.hour - 1;
                            break;
                        case 1:  // 分钟
                            temp_alarm.minute = (temp_alarm.minute == 0) ? 59 : temp_alarm.minute - 1;
                            break;
                        case 2:  // 秒
                            temp_alarm.second = (temp_alarm.second == 0) ? 59 : temp_alarm.second - 1;
                            break;
                        case 3:  // 重复
                            temp_alarm.repeat = !temp_alarm.repeat;
                            break;
                    }
                    Display_Set_Alarm();
                    break;
                    
                case KEY2_PRES:  // 确认/返回
                    // 保存闹钟设置
                    printf("Saving alarm: %02d:%02d:%02d Repeat:%d\n", 
                           temp_alarm.hour, temp_alarm.minute, temp_alarm.second, temp_alarm.repeat);
                           OLED_Clear();
                    return;
                    
                case KEY3_PRES:  // 下一个设置项（循环）
                    set_alarm_step++;
                    if (set_alarm_step >= 4) {
                        set_alarm_step = 0;  // 回到第一项，不是退出
                    }
                    Display_Set_Alarm();
                    break;
            }
        }
    }
}

// 新建闹钟功能实现
void alarm_create(void)
{
    Process_Set_Alarm(NULL);
    
    // 保存新闹钟
    if (Alarm_Add(&temp_alarm) == 0) {
        OLED_Clear();
        OLED_Printf_Line(1, " ALARM SET ");
        OLED_Printf_Line(2, " SUCCESS! ");
        OLED_Refresh();
        delay_ms(1000);
    } else {
        OLED_Clear();
        OLED_Printf_Line(1, " ALARM SET ");
        OLED_Printf_Line(2, " FAILED! ");
        OLED_Refresh();
        delay_ms(1000);
    }
}

/**
 * @brief 显示闹钟详情界面
 */
void Display_Alarm_Detail(u8 alarm_index, u8 selected_option)
{
    if (alarm_index >= g_alarm_count) return;
    
    Alarm_TypeDef* alarm = &g_alarms[alarm_index];
    
    OLED_Printf_Line(0, "%c Time: %02d:%02d:%02d", selected_option == 0 ? '>' : ' ', alarm->hour, alarm->minute, alarm->second);
    OLED_Printf_Line(1, "%c Status: %s", selected_option == 1 ? '>' : ' ', alarm->enabled ? "ENABLED " : "DISABLED");
    OLED_Printf_Line(2, "%c Repeat: %s", selected_option == 2 ? '>' : ' ', alarm->repeat ? "YES" : "NO");
    OLED_Printf_Line(3, "%c Delete", selected_option == 3 ? '>' : ' ');
    OLED_Refresh_Dirty();
}

/**
 * @brief 处理闹钟详情操作
 */
void Process_Alarm_Detail(u8 alarm_index)
{
    if (alarm_index >= g_alarm_count) return;
    
    u8 key;
    u8 selected_option = 0;
    u8 operating = 1;
    
    // 显示闹钟详情
    Display_Alarm_Detail(alarm_index, selected_option);
    
    while (operating) {
        delay_ms(10);
        if ((key = KEY_Get()) != 0) {
            switch (key) {
                case KEY0_PRES:  // 上一个选项
                    selected_option = (selected_option == 0) ? 3 : selected_option - 1;
                    Display_Alarm_Detail(alarm_index, selected_option);
                    break;
                    
                case KEY1_PRES:  // 下一个选项
                    selected_option = (selected_option + 1) % 4;
                    Display_Alarm_Detail(alarm_index, selected_option);
                    break;
                    
                case KEY2_PRES:  // 返回
                    operating = 0;
                    break;
                    
                case KEY3_PRES:  // 确认操作
                    switch (selected_option) {
                        case 0:  // 时间选项 - 进入修改页面
                            {
                                Alarm_TypeDef old_alarm = g_alarms[alarm_index];
                                Process_Set_Alarm(&old_alarm);
                                
                                // 更新闹钟时间
                                Alarm_Delete(alarm_index);
                                Alarm_Add(&temp_alarm);
                                
                                // 重新显示详情界面
                                Display_Alarm_Detail(alarm_index, selected_option);
                            }
                            break;
                            
                        case 1:  // 切换状态
                            if (g_alarms[alarm_index].enabled) {
                                Alarm_Disable(alarm_index);
                            } else {
                                Alarm_Enable(alarm_index);
                            }
                            Display_Alarm_Detail(alarm_index, selected_option);
                            break;
                            
                        case 2:  // 切换重复状态
                            g_alarms[alarm_index].repeat = !g_alarms[alarm_index].repeat;
                            Display_Alarm_Detail(alarm_index, selected_option);
                            break;
                            
                        case 3:  // 删除闹钟
                            Alarm_Delete(alarm_index);
                            OLED_Clear();
                            OLED_Printf_Line(1, " ALARM DELETED ");
                            OLED_Refresh();
                            delay_ms(1000);
                            operating = 0;
                            break;
                    }
                    break;
            }
        }
    }
}

// 闹钟列表功能实现
void alarm_list(void)
{
    u8 key;
    u8 display_start = 0;
    u8 selected = 0;
    u8 flag_cl=1;
    if (g_alarm_count == 0) {
        OLED_Clear();
        OLED_Printf_Line(1, " NO ALARMS ");
        OLED_Printf_Line(2, "   FOUND   ");
        OLED_Refresh();
        delay_ms(1000);
        OLED_Clear();
        return;
    }
    
    while (1) {
 if (flag_cl)
 {
    OLED_Clear();
    flag_cl=0;
 }
 
        delay_ms(10);
        if ((key = KEY_Get()) != 0) {
            switch (key) {
                case KEY0_PRES:  // 上一个
                    if (selected > 0) {
                        selected--;
                    } else if (display_start > 0) {
                        display_start--;
                    }
                    break;
                    
                case KEY1_PRES:  // 下一个
                    if (selected < g_alarm_count - 1) {
                        if (selected < 3) {
                            selected++;
                        } else if (display_start + 4 < g_alarm_count) {
                            display_start++;
                        }
                    }
                    break;
                    
                case KEY2_PRES:  // 返回
                    OLED_Clear();
                    return;
                    
                case KEY3_PRES:  // 进入当前选中闹钟的详情界面
                    Process_Alarm_Detail(display_start + selected);
                    flag_cl=1;
                    // 重新显示列表
                    selected = 0;
                    display_start = 0;
                    break;
            }
        }
        
       
        OLED_Printf_Line(0, "ALARMS LIST");
        
        // 显示最多4个闹钟
        for (u8 i = 0; i < 4 && (display_start + i) < g_alarm_count; i++) {
            Alarm_TypeDef* alarm = &g_alarms[display_start + i];
            OLED_Printf_Line(i + 1, "%c%02d:%02d:%02d %s %s",
                            (i == selected) ? '>' : ' ',
                            alarm->hour, alarm->minute, alarm->second,
                            alarm->enabled ? "ON " : "OFF",
                            alarm->repeat ? "R" : " ");
        }
        OLED_Refresh();
    }
}

void alarm_menu()
{
    u8 flag_RE = 1;
    u8 key;
    u8 selected = 0;
 
    while (1)
    {delay_ms(10);
        if (flag_RE)
        {
            OLED_Clear();
            alarm_menu_Ref(selected);
            flag_RE = 0;
        }
        
        if ((key = KEY_Get()) != 0)
        {
            switch (key)
            {
            case KEY0_PRES:
                if (selected == 0)
                {
                    selected = alarm_menu_options_NUM - 1; // 0→最后一项
                }
                else
                {
                    selected--;
                }
                alarm_menu_Ref(selected);
                break;
                
            case KEY1_PRES:
                selected++;
                selected = selected % alarm_menu_options_NUM;
                alarm_menu_Ref(selected);
                break;
                
            case KEY2_PRES:
                OLED_Clear();
                return;
                
            case KEY3_PRES:
                flag_RE = 1;
                alarm_menu_Enter_select(selected); // 进入所选择的菜单项
                break;

            default:
                break;
            }
        }
    }
}
