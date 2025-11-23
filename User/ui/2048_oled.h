/**
 * @file 2048_oled.h
 * @brief 2048游戏头文件 - 定义游戏接口和相关依赖
 * @author flowkite-0689
 * @email 2604338097@qq.com
 * @version 1.0
 * @date 2024
 * 
 * 本文件定义了2048游戏的主要接口函数，包含必要的头文件依赖
 */

#ifndef _2048_oled_H_
#define _2048_oled_H_

#include "key.h"
#include "oled.h"
#include "oled_print.h"
#include "MPU6050.h"


/**
 * @brief 2048游戏主菜单函数
 * @details 显示游戏菜单，处理用户选择和按键操作
 */
void menu_2048_oled(void);

#endif
