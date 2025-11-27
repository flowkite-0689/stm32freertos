#ifndef _AIR_LEVEL_H_
#define _AIR_LEVEL_H_

#include "MPU6050.h"
#include "key.h"
#include "oled.h"
#include "oled_print.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
void calculate_tilt_angles(short ax, short ay, short az, float *angle_x, float *angle_y);

int getmove(void);
/**
 * @brief 游戏信息存储数组
 * @details 存储游戏相关信息
 * info[0]: 方向信息 (left, right, up, down等)
 * info[1]: 角度信息 (倾斜角度显示)
 * info[2]: 游戏状态信息 (游戏结束等)
 */
extern char *info[3];  // 外部声明，不在这里初始化
extern float iiix;
extern float iiiy;
void init_info(void);
void cleanup_info(void);
void air_level_test(void);
#endif
