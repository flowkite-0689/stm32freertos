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
    uint8_t y = line * OLED_LINE_HEIGHT; // 16
    OLED_Clear_Rect(0, y, 127, y + OLED_LINE_HEIGHT - 1);
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
// 清除指定矩形区域（x1,y1）→（x2,y2），并标记为脏区
 
void OLED_Clear_Rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    if (x1 > x2) { uint8_t t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { uint8_t t = y1; y1 = y2; y2 = t; }
    if (x1 >= 128) x1 = 127; if (x2 >= 128) x2 = 127;
    if (y1 >= 64)  y1 = 63;  if (y2 >= 64)  y2 = 63;

    // 按像素逐点清除（保守但安全）
    for (uint8_t x = x1; x <= x2; x++) {
        for (uint8_t y = y1; y <= y2; y++) {
            OLED_DrawPoint(x, y, 0); // 0 = 清空点
        }
    }

    // 标记脏区
    OLED_Set_Dirty_Area(x1, y1, x2, y2);
}
/**
 * @brief 绘制通用进度条（横向）
 * 
 * @param x, y           : 进度条左上角坐标
 * @param width, height  : 进度条尺寸（不含标签）
 * @param value          : 当前值（如 255）
 * @param min_val        : 最小值（如 0）
 * @param max_val        : 最大值（如 500）
 * @param show_border    : 是否显示边框 (1=是, 0=否)
 * @param fill_mode      : 填充模式 (1=实心, 0=仅外框)
 * 
 * @note 实际绘制区域 = [x, y] ~ [x+width-1, y+height-1]
 *       调用者可自行在外围画标签（如 "0C", "50C"）
 */
void OLED_DrawProgressBar(
    uint8_t x, uint8_t y,
    uint8_t width, uint8_t height,
    int32_t value,
    int32_t min_val, int32_t max_val,
    uint8_t show_border,
    uint8_t fill_mode
) {
    if (width == 0 || height == 0) return;
    if (min_val >= max_val) return;

    // 限制 value 范围
    if (value < min_val) value = min_val;
    if (value > max_val) value = max_val;

    // 计算填充宽度（像素）
    uint32_t range = (uint32_t)(max_val - min_val);
    uint32_t fill_w = (uint32_t)(value - min_val) * width / range;

    // 🟢 关键：先清除整个进度条区域（含旧填充+边框）
    OLED_Clear_Rect(x, y, x + width - 1, y + height - 1);

    // 画边框（可选）
    if (show_border) {
        // 上边
        for (uint8_t i = 0; i < width; i++) {
            OLED_DrawPoint(x + i, y, 1);
        }
        // 下边
        for (uint8_t i = 0; i < width; i++) {
            OLED_DrawPoint(x + i, y + height - 1, 1);
        }
        // 左边
        for (uint8_t i = 0; i < height; i++) {
            OLED_DrawPoint(x, y + i, 1);
        }
        // 右边
        for (uint8_t i = 0; i < height; i++) {
            OLED_DrawPoint(x + width - 1, y + i, 1);
        }
    }

    // 填充内部（可选）
    if (fill_mode) {
        uint8_t x_fill_end = x + fill_w;
        if (x_fill_end > x + width) x_fill_end = x + width;
        for (uint8_t xx = x + (show_border ? 1 : 0);
             xx < x_fill_end - (show_border ? 1 : 0);
             xx++) {
            for (uint8_t yy = y + (show_border ? 1 : 0);
                 yy < y + height - (show_border ? 1 : 0);
                 yy++) {
                OLED_DrawPoint(xx, yy, 1);
            }
        }
    }

    // ✅ 自动标记整个进度条区域为脏（已由 OLED_Clear_Rect 完成）
    // 无需重复调用 OLED_Set_Dirty_Area()
}
