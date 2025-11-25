#include "air_level.h"

// 定义全局共享信息数组
char *info[3] = {NULL, NULL, NULL};
 float iiix=0.0f;
 float iiiy=0.0f;
/**
 * @brief 初始化游戏信息数组
 * @details 为g_game_info数组的每个元素分配内存并初始化为空字符串
 */
void init_info() {
    for(int i = 0; i < 3; i++) {
        if(info[i] != NULL) {
            free(info[i]);
        }
        info[i] = malloc(16);  // 分配足够空间
        strcpy(info[i], "");   // 初始化为空字符串
    }
}

/**
 * @brief 清理游戏信息数组
 * @details 释放info数组分配的内存，避免内存泄漏
 */
void cleanup_info() {
    for(int i = 0; i < 3; i++) {
        if(info[i] != NULL) {
            free(info[i]);
            info[i] = NULL;
        }
    }
}


/**
 * @brief 将MPU6050加速度数据转换为倾斜角度
 * @param ax X轴加速度数据
 * @param ay Y轴加速度数据
 * @param az Z轴加速度数据
 * @param angle_x 计算出的绕Y轴旋转角度(pitch，前后倾斜)
 * @param angle_y 计算出的绕X轴旋转角度(roll，左右倾斜)
 * @details 使用三角函数将三维加速度数据转换为二维倾斜角度
 */
void calculate_tilt_angles(short ax, short ay, short az, float *angle_x, float *angle_y)
{
  // 计算倾斜角度
  // angle_x: 绕Y轴旋转的pitch角度 (前后倾斜)
  // angle_y: 绕X轴旋转的roll角度 (左右倾斜)
  
  // 使用双精度浮点数计算以提高精度
  double ax_d = (double)ax;
  double ay_d = (double)ay;
  double az_d = (double)az;
  
  *angle_x = atan2(ax_d, sqrt(ay_d * ay_d + az_d * az_d)) * 180.0 / 3.14159265;
  *angle_y = atan2(ay_d, sqrt(ax_d * ax_d + az_d * az_d)) * 180.0 / 3.14159265;
}


/**
 * @brief 基于MPU6050体感获取游戏移动方向
 * @return int 返回移动方向：0-左，1-右，2-上，3-下，-1表示无有效移动
 * @details 通过检测设备倾斜角度来确定游戏移动方向，支持方向趋势显示
 */
int 
getmove()
{
    short ax, ay, az;
    static int last_direction = -1;
    static int need_reset = 0;
    int current_direction = -1;
    float angle_x, angle_y;
    
    const float trigger_threshold = 20.0;
    const float reset_threshold = 10.0;
    
    // 确保info数组已初始化
    static int info_initialized = 0;
    if(!info_initialized) {
        init_info();
        info_initialized = 1;
    }
    
    if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0)
    {
        calculate_tilt_angles(ax, ay, az, &angle_x, &angle_y);
        
        char direction_text[10] = "neutral";  // 初始设为中性状态
        float display_angle = 0;
        
        if ((angle_x > 0 ? angle_x : -angle_x) > (angle_y > 0 ? angle_y : -angle_y))
        {
            display_angle = angle_x;
            iiix=angle_x;
            // 无论是否达到触发角度，都设置方向文本
            if(angle_x < -trigger_threshold)
            {
                current_direction = 2;
                strcpy(direction_text, "up   ");
            }
            else if(angle_x > trigger_threshold)
            {
                current_direction = 3;
                strcpy(direction_text, "down ");
            }
            else
            {
                // 未达到触发角度，但显示倾斜趋势
                current_direction = -1;
                if(angle_x < -5.0) {
                    strcpy(direction_text, "up~   ");  // 有向上倾斜趋势
                } else if(angle_x > 5.0) {
                    strcpy(direction_text, "down~"); // 有向下倾斜趋势
                } else {
                    strcpy(direction_text, "flat ");  // 接近水平
                }
            }
        }
        else
        {
            iiiy=angle_y;
            display_angle = angle_y;
            // 无论是否达到触发角度，都设置方向文本
            if(angle_y > trigger_threshold)
            {
                current_direction = 1;
                strcpy(direction_text, "right");
            }
            else if(angle_y < -trigger_threshold)
            {
                current_direction = 0;
                strcpy(direction_text, "left  ");
            }
            else
            {
                // 未达到触发角度，但显示倾斜趋势
                current_direction = -1;
                if(angle_y > 5.0) {
                    strcpy(direction_text, "right~"); // 有向右倾斜趋势
                } else if(angle_y < -5.0) {
                    strcpy(direction_text, "left~  ");  // 有向左倾斜趋势
                } else {
                    strcpy(direction_text, "flat   ");  // 接近水平
                }
            }
        }
        
        // 安全地保存到g_air_level_info数组
        if(info[0] != NULL) {
            strncpy(info[0], direction_text, 15);
            info[0][15] = '\0';
        }
        
        if(info[1] != NULL) {
            snprintf(info[1], 16, " %.1f^     ", display_angle);  // 用'^'替代'°'符号
        }
        
        // 检查是否需要重置
        if(need_reset)
        {
            if((angle_x < reset_threshold && angle_x > -reset_threshold) && 
               (angle_y < reset_threshold && angle_y > -reset_threshold))
            {
                need_reset = 0;
                last_direction = -1;
            }
            else
            {
                return -1;
            }
        }
        
        if(current_direction != -1 && !need_reset)
        {
            need_reset = 1;
            return current_direction;
        }
    }

    return -1;
}


void air_level_test(){
u8 key;
while (1)
{
   
getmove();
  


  
  OLED_Printf_Line(1,"y : %0.1f",iiiy);

  OLED_Printf_Line(2,"x : %0.1f",iiix);




OLED_DrawProgressBar(96,0,2,64,(iiiy*10),-900,900,0,1);


  OLED_DrawProgressBar(64,32,64,2,(iiix*10),-900,900,0,1);









   key = KEY_Get();
    if (key)
    {
      switch (key)
      {
      case KEY2_PRES:

        OLED_Clear();
        return;
      default:
        break;
      }
    }
    IWDG_ReloadCounter();
    OLED_Refresh();
    delay_ms(10);  // 短暂延时，避免过于频繁的查询
}


    
}
