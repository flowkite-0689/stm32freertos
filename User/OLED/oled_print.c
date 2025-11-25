#include "oled_print.h"

// 临时缓冲区用于格式化字符串
static char oled_buffer[128];

/**
 * @brief OLED打印函数 - 在指定位置格式化打印信息
 */
void OLED_Printf(uint8_t x, uint8_t y, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    // 格式化字符串
    vsnprintf(oled_buffer, sizeof(oled_buffer), format, args);
    
    // 显示字符串
    OLED_ShowString(x, y, (uint8_t*)oled_buffer, 12, 1);
    
    va_end(args);
}

/**
 * @brief OLED行打印函数 - 在指定行打印信息
 */
void OLED_Printf_Line(uint8_t line, const char* format, ...)
{
    if (line >= OLED_MAX_LINES) return; // 防止越界
    
    va_list args;
    va_start(args, format);
    
    // 计算Y坐标
    uint8_t y = line * OLED_LINE_HEIGHT;
    
    // 格式化字符串
    vsnprintf(oled_buffer, sizeof(oled_buffer), format, args);
    
    // 清除该行
    OLED_Clear_Line(line);
    
    // 显示字符串
    OLED_ShowString(0, y, (uint8_t*)oled_buffer, 12, 1);
    
    // 标记该行为脏区域，用于局部刷新
    OLED_Set_Dirty_Area(0, y, 127, y + OLED_LINE_HEIGHT - 1);
    
    va_end(args);
}

/**
 * @brief OLED行打印函数32px - 在指定行打印信息
 */
void OLED_Printf_Line_32(uint8_t line, const char* format, ...)
{
    if (line >= OLED_MAX_LINES) return; // 防止越界
    
    va_list args;
    va_start(args, format);
    
    // 计算Y坐标
    uint8_t y = line * OLED_LINE_HEIGHT;
    
    // 格式化字符串
    vsnprintf(oled_buffer, sizeof(oled_buffer), format, args);
    
    // 清除该行
    OLED_Clear_Line(line);
    
    // 显示字符串
    OLED_ShowString(0, y, (uint8_t*)oled_buffer, 24, 1);
    
    // 标记该行为脏区域，用于局部刷新
    OLED_Set_Dirty_Area(0, y, 127, y + (OLED_LINE_HEIGHT*2) - 1);
    
    va_end(args);
}


/**
 * @brief OLED清屏指定行
 */
void OLED_Clear_Line(uint8_t line)
{
    if (line >= OLED_MAX_LINES) return;
    
    uint8_t y = line * OLED_LINE_HEIGHT;
    
    // 绘制空格填充该行
    for (uint8_t x = 0; x < OLED_MAX_CHARS; x++) {
        OLED_ShowChar(x * 8, y, ' ', 12, 1);
    }
}

/**
 * @brief OLED显示系统信息
 */
void OLED_Display_Info(uint8_t mode, const char* info)
{
    // 清屏
    OLED_Clear();
    
    // 第一行显示模式
    OLED_Printf_Line(0, "Mode: %d", mode);
    
    // 第二行显示信息
    if (strlen(info) > OLED_MAX_CHARS) {
        // 信息太长，分两行显示
        char temp[OLED_MAX_CHARS + 1];
        strncpy(temp, info, OLED_MAX_CHARS);
        temp[OLED_MAX_CHARS] = '\0';
        OLED_Printf_Line(1, "%s", temp);
        
        if (strlen(info) > OLED_MAX_CHARS) {
            OLED_Printf_Line(2, "%s", info + OLED_MAX_CHARS);
        }
    } else {
        OLED_Printf_Line(1, "%s", info);
    }
}

/**
 * @brief OLED显示传感器数据
 */
void OLED_Display_Sensor(const char* sensor_name, float data1, float data2, const char* unit)
{
    // 清屏
    OLED_Clear();
    
    // 第一行显示传感器名称
    OLED_Printf_Line(0, "%s", sensor_name);
    
    // 第二行显示数据1
    OLED_Printf_Line(1, "Data1: %.2f %s", data1, unit);
    
    // 第三行显示数据2
    OLED_Printf_Line(2, "Data2: %.2f %s", data2, unit);
    
    // 第四行显示状态
    OLED_Printf_Line(3, "Status: Active");
}
// 横向温度计：显示在 line=1 (y=16~31)，纯方柱，无指针头
void OLED_DrawTempBar_Line1(int16_t temp_tenth)  // 单位：0.1°C，如 255 = 25.5°C
{
    const uint8_t y_top = 16 + 2;     // 起始Y，留2像素边距
    const uint8_t bar_height = 8;     // 进度条高度
    const uint8_t x_start = 18;       // 左侧留空给 "0C"
    const uint8_t x_end = 104;        // 右侧留空给 "50C"
    const uint8_t bar_width = x_end - x_start; // 88 像素

    // 温度范围：0.0°C ~ 50.0°C（可按需改为 -10~60）
    const int16_t min_temp_tenth = 0;   // 0.0°C
    const int16_t max_temp_tenth = 500; // 50.0°C
    const int16_t range = max_temp_tenth - min_temp_tenth; // 500

    // 限制范围
    if (temp_tenth < min_temp_tenth) temp_tenth = min_temp_tenth;
    if (temp_tenth > max_temp_tenth) temp_tenth = max_temp_tenth;

    // 计算填充宽度（像素）
    uint16_t fill_w = (uint32_t)(temp_tenth - min_temp_tenth) * bar_width / range;

    // 清空本行
    OLED_Clear_Line(1);

    // ① 显示刻度标签
    OLED_ShowString(0, 16, (uint8_t*)"0C", 12, 1);
    OLED_ShowString(110, 16, (uint8_t*)"50C", 12, 1);

    // ② 画外框（单像素线）
    OLED_DrawLine(x_start, y_top, x_end, y_top, 1);              // 上
    OLED_DrawLine(x_start, y_top + bar_height - 1, x_end, y_top + bar_height - 1, 1); // 下
    OLED_DrawLine(x_start, y_top, x_start, y_top + bar_height - 1, 1); // 左
    OLED_DrawLine(x_end, y_top, x_end, y_top + bar_height - 1, 1);     // 右

    // ③ ✅ 纯方柱填充（实心矩形，无指针）
    for (uint16_t x = x_start; x < x_start + fill_w && x < x_end; x++) {
        for (uint8_t dy = 1; dy < bar_height - 1; dy++) { // 内部填充，避开边框
            OLED_DrawPoint(x, y_top + dy, 1);
        }
    }

    // 标记脏区域
    OLED_Set_Dirty_Area(0, 16, 127, 31);
}
// 横向湿度条：显示在 line=3 (y=48~63)，纯方柱
void OLED_DrawHumidityBar_Line3(uint8_t humi_percent)
{
    const uint8_t y_top = 48 + 4;     // 居中于 48~63
    const uint8_t bar_height = 8;
    const uint8_t x_start = 18;
    const uint8_t x_end = 104;
    const uint8_t bar_width = x_end - x_start;

    if (humi_percent > 100) humi_percent = 100;
    uint16_t fill_w = (uint32_t)humi_percent * bar_width / 100;

    OLED_Clear_Line(3);

    // ① 标签
    OLED_ShowString(0, 48, (uint8_t*)"0%", 12, 1);
    OLED_ShowString(106, 48, (uint8_t*)"100%", 12, 1);

    // ② 外框
    OLED_DrawLine(x_start, y_top, x_end, y_top, 1);
    OLED_DrawLine(x_start, y_top + bar_height - 1, x_end, y_top + bar_height - 1, 1);
    OLED_DrawLine(x_start, y_top, x_start, y_top + bar_height - 1, 1);
    OLED_DrawLine(x_end, y_top, x_end, y_top + bar_height - 1, 1);

    // ③ ✅ 纯方柱填充（实心，无指针）
    for (uint16_t x = x_start; x < x_start + fill_w && x < x_end; x++) {
        for (uint8_t dy = 1; dy < bar_height - 1; dy++) {
            OLED_DrawPoint(x, y_top + dy, 1);
        }
    }

    OLED_Set_Dirty_Area(0, 48, 127, 63);
}
