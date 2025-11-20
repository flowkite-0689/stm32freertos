// oled_simulator_enhanced.c - 增强版OLED模拟器，支持所有OLED功能
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include "../logo.h"
#include "../OLED/oledfont.h"

// OLED显示屏参数
#define SCALE 4
#define WIDTH 128
#define HEIGHT 64

// 模拟器参数
#define OLED_MAX_LINES 5
#define OLED_LINE_HEIGHT 12
#define OLED_MAX_CHARS 21
#define OLED_CMD 0
#define OLED_DATA 1

// 全局变量
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint8_t OLED_GRAM[144][8];  // 模拟OLED显存
static uint8_t dirty_flag = 0;
static uint8_t dirty_x1 = 127, dirty_y1 = 63, dirty_x2 = 0, dirty_y2 = 0;
static char oled_buffer[128];  // OLED_Print用缓冲区

// 底层I/O函数模拟
void delay_ms(uint32_t ms) {
    SDL_Delay(ms);
}

// 模拟I2C初始化
void OLED_I2C_Init(void) {
    printf("OLED_I2C_Init: 模拟I2C初始化完成\n");
}

// 模拟发送一个字节
void OLED_Send_Byte(uint8_t addr, uint8_t cmd, uint8_t dat) {
    // 模拟发送过程，实际不操作硬件
    // printf("OLED_Send_Byte: addr=0x%02X, cmd=0x%02X, dat=0x%02X\n", addr, cmd, dat);
}

// 模拟发送多个字节
void OLED_Send_Bytes(uint8_t addr, uint8_t cmd, uint8_t len, uint8_t* data) {
    for (uint8_t i = 0; i < len; i++) {
        OLED_Send_Byte(addr, cmd, data[i]);
    }
}

// 写入一个字节
void OLED_WR_Byte(uint8_t dat, uint8_t mode) {
    if (mode) {
        OLED_Send_Byte(0x3c, 0x40, dat); 
    } else {
        OLED_Send_Byte(0x3c, 0x00, dat); 
    }
}

// 颜色翻转函数
void OLED_ColorTurn(uint8_t i) {
    if (i == 0) {
        OLED_WR_Byte(0xA6, OLED_CMD); // 正常显示
        printf("OLED: 正常显示模式\n");
    }
    if (i == 1) {
        OLED_WR_Byte(0xA7, OLED_CMD); // 反色显示
        printf("OLED: 反色显示模式\n");
    }
}

// 屏幕翻转180度
void OLED_DisplayTurn(uint8_t i) {
    if (i == 0) {
        OLED_WR_Byte(0xC8, OLED_CMD); // 正常显示
        OLED_WR_Byte(0xA1, OLED_CMD);
        printf("OLED: 正常显示方向\n");
    }
    if (i == 1) {
        OLED_WR_Byte(0xC0, OLED_CMD); // 反转显示
        OLED_WR_Byte(0xA0, OLED_CMD);
        printf("OLED: 屏幕旋转180度\n");
    }
}

// 开启OLED显示
void OLED_DisPlay_On(void) {
    OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
    OLED_WR_Byte(0x14, OLED_CMD); // 开启电荷泵
    OLED_WR_Byte(0xAF, OLED_CMD); // 点亮屏幕
    printf("OLED: 显示已开启\n");
}

// 关闭OLED显示
void OLED_DisPlay_Off(void) {
    OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
    OLED_WR_Byte(0x10, OLED_CMD); // 关闭电荷泵
    OLED_WR_Byte(0xAE, OLED_CMD); // 关闭屏幕
    printf("OLED: 显示已关闭\n");
}

// 刷新显示
void OLED_Refresh(void) {
    printf("OLED: 全屏刷新\n");
    // 在模拟器中，这由SDL渲染系统处理
}

// 局部刷新
void OLED_Refresh_Area(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    printf("OLED: 局部刷新 [%d,%d] to [%d,%d]\n", x1, y1, x2, y2);
    // 在模拟器中，这由SDL渲染系统处理
}

// 设置脏区域
void OLED_Set_Dirty_Area(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    // 校正坐标
    if (x1 > x2) { uint8_t temp = x1; x1 = x2; x2 = temp; }
    if (y1 > y2) { uint8_t temp = y1; y1 = y2; y2 = temp; }
    
    // 边界检查
    if (x1 >= 128) x1 = 127;
    if (x2 >= 128) x2 = 127;
    if (y1 >= 64) y1 = 63;
    if (y2 >= 64) y2 = 63;
    
    // 更新脏区域
    if (!dirty_flag) {
        dirty_x1 = x1; dirty_y1 = y1; dirty_x2 = x2; dirty_y2 = y2;
        dirty_flag = 1;
    } else {
        if (x1 < dirty_x1) dirty_x1 = x1;
        if (y1 < dirty_y1) dirty_y1 = y1;
        if (x2 > dirty_x2) dirty_x2 = x2;
        if (y2 > dirty_y2) dirty_y2 = y2;
    }
}

// 刷新脏区域
void OLED_Refresh_Dirty(void) {
    if (dirty_flag) {
        OLED_Refresh_Area(dirty_x1, dirty_y1, dirty_x2, dirty_y2);
        dirty_flag = 0;
        dirty_x1 = 127; dirty_y1 = 63; dirty_x2 = 0; dirty_y2 = 0;
    }
}

// 清屏
void OLED_Clear(void) {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t n = 0; n < 128; n++) {
            OLED_GRAM[n][i] = 0; // 清除所有数据
        }
    }
    printf("OLED: 清屏完成\n");
}

// 画点
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t) {
    if (x >= WIDTH || y >= HEIGHT) return;
    
    uint8_t i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    if (t) {
        OLED_GRAM[x][i] |= n;
    } else {
        OLED_GRAM[x][i] &= ~n;
    }
}

// 画线
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode) {
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0)
        incx = 1;
    else if (delta_x == 0)
        incx = 0;
    else {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0;
    else {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    for (t = 0; t < distance + 1; t++) {
        OLED_DrawPoint(uRow, uCol, mode);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

// 画圆
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r) {
    int a, b, num;
    a = 0;
    b = r;
    while (2 * b * b >= r * r) {
        OLED_DrawPoint(x + a, y - b, 1);
        OLED_DrawPoint(x - a, y - b, 1);
        OLED_DrawPoint(x - a, y + b, 1);
        OLED_DrawPoint(x + a, y + b, 1);

        OLED_DrawPoint(x + b, y + a, 1);
        OLED_DrawPoint(x + b, y - a, 1);
        OLED_DrawPoint(x - b, y - a, 1);
        OLED_DrawPoint(x - b, y + a, 1);

        a++;
        num = (a * a + b * b) - r * r;
        if (num > 0) {
            b--;
            a--;
        }
    }
}

// 显示字符
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode) {
    uint8_t i, m, temp, size2, chr1;
    uint8_t x0 = x, y0 = y;
    if (size1 == 8)
        size2 = 6;
    else
        size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2);
    chr1 = chr - ' ';
    for (i = 0; i < size2; i++) {
        if (size1 == 8) {
            temp = asc2_0806[chr1][i];
        } else if (size1 == 12) {
            temp = asc2_1206[chr1][i];
        } else if (size1 == 16) {
            temp = asc2_1608[chr1][i];
        } else if (size1 == 24) {
            temp = asc2_2412[chr1][i];
        } else
            return;
        for (m = 0; m < 8; m++) {
            if (temp & 0x01)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp >>= 1;
            y++;
        }
        x++;
        if ((size1 != 8) && ((x - x0) == size1 / 2)) {
            x = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}

// 显示字符串
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode) {
    while ((*chr >= ' ') && (*chr <= '~')) {
        OLED_ShowChar(x, y, *chr, size1, mode);
        if (size1 == 8)
            x += 6;
        else
            x += size1 / 2;
        chr++;
    }
}

// m^n
uint32_t OLED_Pow(uint8_t m, uint8_t n) {
    uint32_t result = 1;
    while (n--) {
        result *= m;
    }
    return result;
}

// 显示数字
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode) {
    uint8_t t, temp, m = 0;
    if (size1 == 8)
        m = 2;
    for (t = 0; t < len; t++) {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;
        if (temp == 0) {
            OLED_ShowChar(x + (size1 / 2 + m) * t, y, '0', size1, mode);
        } else {
            OLED_ShowChar(x + (size1 / 2 + m) * t, y, temp + '0', size1, mode);
        }
    }
}

// 显示中文
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode) {
    uint8_t m, temp;
    uint8_t x0 = x, y0 = y;
    uint16_t i, size3 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * size1;
    for (i = 0; i < size3; i++) {
        if (size1 == 16) {
            temp = Hzk1[num][i];
        } else if (size1 == 24) {
            temp = Hzk2[num][i];
        } else if (size1 == 32) {
            temp = Hzk3[num][i];
        } else if (size1 == 64) {
            temp = Hzk4[num][i];
        } else
            return;
        for (m = 0; m < 8; m++) {
            if (temp & 0x01)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp >>= 1;
            y++;
        }
        x++;
        if ((x - x0) == size1) {
            x = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}

// 显示图片
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, const uint8_t BMP[], uint8_t mode) {
    uint16_t j = 0;
    uint8_t i, n, temp, m;
    uint8_t x0 = x, y0 = y;
    sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);
    for (n = 0; n < sizey; n++) {
        for (i = 0; i < sizex; i++) {
            temp = BMP[j];
            j++;
            for (m = 0; m < 8; m++) {
                if (temp & 0x01)
                    OLED_DrawPoint(x, y, mode);
                else
                    OLED_DrawPoint(x, y, !mode);
                temp >>= 1;
                y++;
            }
            x++;
            if ((x - x0) == sizex) {
                x = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
}

// OLED初始化
void OLED_Init(void) {
    printf("OLED: 初始化中...\n");
    
    OLED_I2C_Init();

    OLED_WR_Byte(0xAE, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    OLED_WR_Byte(0x40, OLED_CMD);
    OLED_WR_Byte(0x81, OLED_CMD);
    OLED_WR_Byte(0xCF, OLED_CMD);
    OLED_WR_Byte(0xA1, OLED_CMD);
    OLED_WR_Byte(0xC8, OLED_CMD);
    OLED_WR_Byte(0xA6, OLED_CMD);
    OLED_WR_Byte(0xA8, OLED_CMD);
    OLED_WR_Byte(0x3f, OLED_CMD);
    OLED_WR_Byte(0xD3, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0xd5, OLED_CMD);
    OLED_WR_Byte(0x80, OLED_CMD);
    OLED_WR_Byte(0xD9, OLED_CMD);
    OLED_WR_Byte(0xF1, OLED_CMD);
    OLED_WR_Byte(0xDA, OLED_CMD);
    OLED_WR_Byte(0x12, OLED_CMD);
    OLED_WR_Byte(0xDB, OLED_CMD);
    OLED_WR_Byte(0x30, OLED_CMD);
    OLED_WR_Byte(0x20, OLED_CMD);
    OLED_WR_Byte(0x02, OLED_CMD);
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_Clear();
    OLED_WR_Byte(0xAF, OLED_CMD);
    
    printf("OLED: 初始化完成\n");
}

// ============= OLED_Print功能 =============

/**
 * @brief OLED打印函数 - 在指定位置格式化打印信息
 */
void OLED_Printf(uint8_t x, uint8_t y, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // 格式化字符串
    vsnprintf(oled_buffer, sizeof(oled_buffer), format, args);
    
    // 显示字符串
    OLED_ShowString(x, y, (uint8_t*)oled_buffer, 12, 1);
    
    va_end(args);
}

// 函数声明
void OLED_Clear_Line(uint8_t line);

/**
 * @brief OLED行打印函数 - 在指定行打印信息
 */
void OLED_Printf_Line(uint8_t line, const char* format, ...) {
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
 * @brief OLED清屏指定行
 */
void OLED_Clear_Line(uint8_t line) {
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
void OLED_Display_Info(uint8_t mode, const char* info) {
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
void OLED_Display_Sensor(const char* sensor_name, float data1, float data2, const char* unit) {
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

// 滚动显示
void OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode) {
    uint8_t i, n, t = 0, m = 0, r;
    while (1) {
        if (m == 0) {
            OLED_ShowChinese(128, 24, t, 16, mode);
            t++;
        }
        if (t == num) {
            for (r = 0; r < 16 * space; r++) {
                for (i = 1; i < 144; i++) {
                    for (n = 0; n < 8; n++) {
                        OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
                    }
                }
                OLED_Refresh_Dirty();
            }
            t = 0;
        }
        m++;
        if (m == 16) {
            m = 0;
        }
        for (i = 1; i < 144; i++) {
            for (n = 0; n < 8; n++) {
                OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
            }
        }
        OLED_Refresh_Dirty();
    }
}

// ============= SDL模拟器相关 =============

// 初始化SDL窗口
void oled_simulator_init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        exit(1);
    }
    
    window = SDL_CreateWindow("OLED增强模拟器", 
                              SDL_WINDOWPOS_CENTERED, 
                              SDL_WINDOWPOS_CENTERED,
                              WIDTH * SCALE, 
                              HEIGHT * SCALE, 
                              SDL_WINDOW_SHOWN);
    if (!window) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        exit(1);
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        exit(1);
    }
    
    SDL_RenderSetScale(renderer, SCALE, SCALE);
    
    // 清空OLED显存
    memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
    
    printf("OLED模拟器初始化完成\n");
}

// 保存OLED内容为文本文件（用于调试）
void save_oled_to_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("无法创建文件: %s\n", filename);
        return;
    }
    
    fprintf(file, "OLED GRAM内容 (%dx%d):\n", WIDTH, HEIGHT);
    fprintf(file, "================================\n");
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int bit_mask = 1 << (y % 8);
            if (OLED_GRAM[x][y / 8] & bit_mask) {
                fprintf(file, "█");
            } else {
                fprintf(file, " ");
            }
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    printf("OLED内容已保存到: %s\n", filename);
}

// 更新SDL显示
void oled_simulator_update_display() {
    // 清屏
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // 绘制像素
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            int byte_index = x + (y / 8) * WIDTH;
            int bit_mask = 1 << (y % 8);
            
            if (OLED_GRAM[x][y / 8] & bit_mask) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
    
    SDL_RenderPresent(renderer);
}

// 清理SDL资源
void oled_simulator_cleanup() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
    printf("OLED模拟器已清理\n");
}

// OLED演示函数
void oled_demo() {
    extern const unsigned char logo[1024];
    uint8_t t = ' ';
    OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
   
    OLED_ShowPicture(32, 0, 64, 64, gImage_bg, 1);
    printf("已显示开机图像，等待2秒...\n");
    oled_simulator_update_display();
    save_oled_to_file("boot_image.txt");
    delay_ms(2000);
    OLED_Clear();

    OLED_ShowChinese(0 + 6, 0, 0, 16, 1);
    OLED_ShowChinese(18 + 6, 0, 1, 16, 1);
    OLED_ShowChinese(36 + 6, 0, 2, 16, 1);
    OLED_ShowChinese(54 + 6, 0, 3, 16, 1);
    OLED_ShowChinese(72 + 6, 0, 4, 16, 1);
    OLED_ShowChinese(90 + 6, 0, 5, 16, 1);
    OLED_ShowString(8, 16, (uint8_t*)"ZHUANGZHOU", 16, 1);
    OLED_ShowString(20, 32, (uint8_t*)"2023/03/01", 16, 1);
    OLED_ShowString(0, 48, (uint8_t*)"ASCII:", 16, 1);
    OLED_ShowString(63, 48, (uint8_t*)"CODE:", 16, 1);
    OLED_ShowChar(48, 48, t, 16, 1);
    t++;
    if (t > '~')
        t = ' ';
    OLED_ShowNum(103, 48, t, 3, 16, 1);
    delay_ms(2000);
    OLED_Clear();
    OLED_ShowChinese(0, 0, 3, 16, 1);
    OLED_ShowChinese(16, 0, 0, 24, 1);
    OLED_ShowChinese(24, 20, 0, 32, 1);
    OLED_ShowChinese(64, 0, 0, 64, 1);
    delay_ms(2000);
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"ABC", 8, 1);
    OLED_ShowString(0, 8, (uint8_t*)"ABC", 12, 1);
    OLED_ShowString(0, 20, (uint8_t*)"ABC", 16, 1);
    OLED_ShowString(0, 36, (uint8_t*)"ABC", 24, 1);
    delay_ms(2000);
    
    // 演示OLED_Printf功能
    OLED_Clear();
    OLED_Printf(10, 10, "Hello, OLED!");
    OLED_Printf_Line(1, "Line 1: System OK");
    OLED_Printf_Line(2, "Line 2: %.2f%%", 85.67);
    OLED_Printf_Line(3, "Time: %dms", SDL_GetTicks());
    delay_ms(3000);
    
    // 演示OLED_Display_Sensor
    OLED_Display_Sensor("Temperature", 25.6, 18.2, "C");
    delay_ms(3000);
    
    // 演示OLED_Display_Info
    OLED_Display_Info(2, "Enhanced OLED Simulator with full feature support");
    delay_ms(3000);
}

int main() {
    printf("启动OLED增强模拟器...\n");
    
    oled_simulator_init();
    
    // 运行演示
    oled_demo();
    
    // 清屏显示结束信息
    OLED_Clear();
    OLED_ShowString(20, 20, (uint8_t*)"Demo Complete!", 16, 1);
    OLED_ShowString(30, 40, (uint8_t*)"Press X to exit", 12, 1);
    
    // 主循环
    printf("进入主循环，等待用户关闭窗口或按X键...\n");
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x)) {
                printf("检测到退出事件\n");
                running = 0;
            }
        }
        
        oled_simulator_update_display();
        SDL_Delay(16); // 约60FPS
    }
    
    oled_simulator_cleanup();
    return 0;
}