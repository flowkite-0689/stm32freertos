/**
 * @file 2048_oled.c
 * @brief 2048游戏实现文件 - 支持OLED显示和MPU6050体感控制
 * @author flowkite-0689
 * @email 2604338097@qq.com
 * @version 1.0
 * @date 2025-11-23
 * 
 * 本文件实现了2048游戏的完整逻辑，包括：
 * - 游戏核心玩法：数字移动、合并、得分
 * - OLED显示：适配128x64屏幕的界面布局
 * - MPU6050体感控制：通过倾斜角度控制游戏方向
 * - 用户界面：菜单系统和游戏状态显示
 */

/*
 * OLED屏幕布局分析 (128x64像素)
 * 
 * 显示参数（来自oled_print.h）:
 * - OLED_LINE_HEIGHT = 16像素/行
 * - OLED_MAX_LINES = 4行 (16*4=64像素)
 * - OLED_MAX_CHARS = 16字符/行 (8像素宽字符，16*8=128像素)
 * - 字体大小: 12像素宽x16像素高 (使用OLED_ShowString的12号字体)
 * 
 * 游戏界面布局:
 * ┌─────────────────────────────────────────┬─────────────────┐
 * │ 第0行 (y:0-15):  棋盘第1行              │ 分数显示        │
 * │         ".  .  .  ."                    │ score:100       │
 * ├─────────────────────────────────────────┼─────────────────┤
 * │ 第1行 (y:16-31): 棋盘第2行 ".  .  .  ." │ 方向显示        │
 * │                                         │ right           │
 * ├─────────────────────────────────────────┼─────────────────┤
 * │ 第2行 (y:32-47): 棋盘第3行 ".  .  .  ." │ 角度显示        │
 * │                                         │ 30°             │
 * ├─────────────────────────────────────────┴─────────────────┤
 * │ 第3行 (y:48-63): 棋盘第4行 + 游戏状态                     │
 * └─────────────────────────────────────────────────────────┘
 * 
 * 棋盘布局详细分析:
 * - 每个格子字符: 1字符(12像素) + 2空格(24像素) = 36像素
 * - 4个格子: 36*4 = 144像素 > 128像素 (会超出屏幕)
 * - 实际布局: 每个格子简化为1字符+1空格 = 24像素
 * - 4个格子: 24*4 = 96像素 (x坐标0-95)
 * - 右侧信息区: 128-96 = 32像素 (x坐标96-127)
 *   格式: "C "  (C=棋盘字符, 后跟1空格)
 * 
 * 第0行内容分析:
 * - 棋盘部分: ". . . " (4x2=8字符 = 96像素，使用12像素字体)
 * - 分数部分: 显示在右侧32像素区域
 * 
 * 右侧信息显示:
 * - 第0行右侧: 分数显示 (x:96-127, y:0-15)
 *   格式: "sc:分" (简洁格式)
 * - 第1行右侧: 方向显示 (x:96-127, y:16-31)  
 *   格式: 
 *     触发状态: "right", "left", "up", "down"
 *     倾斜趋势: "right~", "left~", "up~", "down~" (未达到25度，但>5度)
 *     水平状态: "flat" (所有角度<5度)
 * - 第2行右侧: 角度显示 (x:96-127, y:32-47)
 *   格式: "30^", "15.5^" 等 (使用'^'替代'°'符号，因为12号字体中没有度数符号)
 * - 第3行: 完整用于棋盘显示，无右侧信息区
 * 
 * 布局方案:
 * - 第0行: ". . . " + "sc:100"
 * - 第1行: ". . . " + "right"
 * - 第2行: ". . . " + "30°"  
 * - 第3行: ". . .  OVER" (完整显示)
 */

#include "2048_oled.h"
#include "oled_print.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#define SIZE 4

/**
 * @brief 游戏棋盘数组
 * @details 4x4的二维数组，存储游戏棋盘上的数字
 */
int board[SIZE][SIZE];

/**
 * @brief 游戏信息存储数组
 * @details 存储游戏相关信息
 * info[0]: 方向信息 (left, right, up, down等)
 * info[1]: 角度信息 (倾斜角度显示)
 * info[2]: 游戏状态信息 (游戏结束等)
 */
char *info[3] = {NULL, NULL, NULL};  // 初始化为NULL指针

/**
 * @brief 游戏得分
 * @details 记录玩家当前的总得分
 */
int score = 0;

/**
 * @brief 游戏结束标志
 * @details 0表示游戏进行中，1表示游戏结束
 */
int flagisgameover = 0;


/**
 * @brief 初始化游戏信息数组
 * @details 为info数组的每个元素分配内存并初始化为空字符串
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
 * @brief 在棋盘的随机空位上添加新数字
 * @details 遍历棋盘找到所有空位置，随机选择一个位置添加2或4
 *          90%概率添加2，10%概率添加4
 */
void addNum()
{
  int empty[SIZE * SIZE][2];
  int count = 0;
  for (int i = 0; i < SIZE; i++)
  {
    for (int j = 0; j < SIZE; j++)
    {
      if (board[i][j] == 0)
      {
        empty[count][0] = i;
        empty[count][1] = j;
        count++;
      }
    }
  }
  if (count)
  {
    int index = rand() % count;
    int num = (rand() % 10 < 9) ? 2 : 4;
    board[empty[index][0]][empty[index][1]] = num;
  }
}

/**
 * @brief 初始化游戏棋盘
 * @details 清空棋盘并在随机位置添加两个初始数字
 */
void init()
{

  for (int i = 0; i < SIZE; i++)
  {
    for (int j = 0; j < SIZE; j++)
    {
      board[i][j] = 0;
    }
  }
  addNum();
  addNum();
}

/**
 * @brief 游戏数字移动和合并逻辑
 * @param direction 移动方向：0-左，1-右，2-上，3-下
 * @return int 返回是否发生移动：1表示有移动，0表示无移动
 * @details 实现数字的移动和合并，计算得分，防止同一数字重复合并
 */
int move(int direction)
{
  /*
  0 左
  1 右
  2 上
  3 下
  */

  int he[SIZE * SIZE / 2][2]; // 已合并的位置
  int count = 0;
  int moved = 0;      // 判断有无移动
  if (direction == 0) // 左移
  {
    for (int i = 0; i < SIZE; i++)
    {
      for (int j = 1; j < SIZE; j++)
      {
        if (board[i][j] != 0)
        {
          int k = j;
          while (k > 0 && board[i][k - 1] == 0)
          {
            board[i][k - 1] = board[i][k];
            board[i][k] = 0;
            k--;
            moved = 1;
          }
          if (k > 0 && board[i][k - 1] == board[i][k])
          {
            int flag = 0;
            for (int z = 0; z < count; z++)
            {
              if (he[z][0] == i && he[z][1] == (k - 1))
              {
                flag = 1; // 判断已合并过
              }
            }
            if (!flag)
            {
              board[i][k - 1] *= 2;
              he[count][0] = i;
              he[count][1] = (k - 1);
              count++;
              score += board[i][k - 1];
              board[i][k] = 0;
              moved = 1;
            }
          }
        }
      }
    }
  }
  else if (direction == 1) // 右移
  {
    for (int i = 0; i < SIZE; i++)
    {
      for (int j = SIZE - 2; j >= 0; j--)
      {
        if (board[i][j] != 0)
        {
          int k = j;
          while (k < SIZE - 1 && board[i][k + 1] == 0)
          {
            board[i][k + 1] = board[i][k];
            board[i][k] = 0;
            k++;
            moved = 1;
          }
          if (k < SIZE - 1 && board[i][k + 1] == board[i][k])
          {
            int flag = 0;
            for (int z = 0; z < count; z++)
            {
              if (he[z][0] == i && he[z][1] == (k + 1))
              {
                flag = 1; // 判断已合并过
              }
            }
            if (!flag)
            {
              board[i][k + 1] *= 2;
              score += board[i][k + 1];
              he[count][0] = i;
              he[count][1] = (k + 1);
              count++;
              board[i][k] = 0;
              moved = 1;
            }
          }
        }
      }
    }
  }
  else if (direction == 2) // 上移
  {
    for (int j = 0; j < SIZE; j++)
    {
      for (int i = 1; i < SIZE; i++)
      {
        if (board[i][j] != 0)
        {
          int k = i;
          while (k > 0 && board[k - 1][j] == 0)
          {
            board[k - 1][j] = board[k][j];
            board[k][j] = 0;
            k--;
            moved = 1;
          }
          if (k > 0 && board[k - 1][j] == board[k][j])
          {
            int flag = 0;
            for (int z = 0; z < count; z++)
            {
              if (he[z][0] == (k - 1) && he[z][1] == j)
              {
                flag = 1; // 判断已合并过
              }
            }
            if (!flag)
            {
              board[k - 1][j] *= 2;
              score += board[k - 1][j];
              he[count][0] = (k - 1);
              he[count][1] = j;
              count++;
              board[k][j] = 0;
              moved = 1;
            }
          }
        }
      }
    }
  }
  else if (direction == 3) // 下移
  {
    for (int j = 0; j < SIZE; j++)
    {
      for (int i = SIZE - 2; i >= 0; i--)
      {
        if (board[i][j] != 0)
        {
          int k = i;
          while (k < SIZE - 1 && board[k + 1][j] == 0)
          {
            board[k + 1][j] = board[k][j];
            board[k][j] = 0;
            k++;
            moved = 1;
          }
          if (k < SIZE - 1 && board[k + 1][j] == board[k][j])
          {
            int flag = 0;
            for (int z = 0; z < count; z++)
            {
              if (he[z][0] == (k + 1) && he[z][1] == j)
              {
                flag = 1; // 判断已合并过
              }
            }
            if (!flag)
            {
              board[k + 1][j] *= 2;
              score += board[k + 1][j];
              he[count][0] = (k + 1);
              he[count][1] = j;
              count++;
              board[k][j] = 0;
              moved = 1;
            }
          }
        }
      }
    }
  }
  return moved;
}

/**
 * @brief 检查游戏是否结束
 * @return int 返回游戏状态：1表示游戏结束，0表示游戏继续
 * @details 检查是否达到2048获胜条件，或者棋盘满且无法继续移动
 */
int isGameover()
{

  for (int i = 0; i < SIZE; i++)
  {
    for (int j = 0; j < SIZE; j++)
    {
      if (board[i][j] == 2048)
      {
        printf("合成了2048\nWe are the champion!!!\n");
        return 1;
      }
    }
  }
  for (int i = 0; i < SIZE; i++)
  {
    for (int j = 0; j < SIZE; j++)
    {

      if (!board[i][j])
      {
        return 0;
      }
    }
  }
  for (int i = 0; i < SIZE - 1; i++)
  {
    for (int j = 0; j < SIZE; j++)
    {
      if (board[i + 1][j] == board[i][j])
      {
        return 0;
      }
    }
  }

  for (int i = 0; i < SIZE; i++)
  {
    for (int j = 0; j < SIZE - 1; j++)
    {
      if (board[i][j + 1] == board[i][j])
      {
        return 0;
      }
    }
  }

  return 1;
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
        
        // 安全地保存到info数组
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

// 游戏运行界面

/**
 * @brief 在OLED屏幕上显示游戏棋盘
 * @details 将数字转换为对应字符显示在OLED屏幕上，同时显示分数、方向、角度等信息
 *          数字对应关系：2-9→数字字符，16→'A', 32→'B', 64→'C'等
 */
void printBoard()
{
    // 安全地设置游戏结束状态
    if(info[2] != NULL) {
        strncpy(info[2], flagisgameover ? "over" : " ", 15);
        info[2][15] = '\0';
    }
    
    for(int i = 0; i < SIZE; i++)
    {
        char c[SIZE];
        for(int j = 0; j < SIZE; j++)
        {
            int number = board[i][j];
            if(number < 10)
            {
                c[j] = number + '0';
            }
            else if(number == 16)
            {
                c[j] = 'A';
            }
            else if(number == 32)
            {
                c[j] = 'B';
            }
            else if(number == 64)
            {
                c[j] = 'C';
            }
            else if(number == 128)
            {
                c[j] = 'D';
            }
            else if(number == 256)
            {
                c[j] = 'E';
            }
            else if(number == 512)
            {
                c[j] = 'F';
            }
            else if(number == 1024)
            {
                c[j] = 'G';
            }
            else if(number == 2048)
            {
                c[j] = 'H';
            }
            else
            {
                c[j] = '.';
            }
        }

        if(i == 0)
        {
            OLED_Printf_Line(i, "%c  %c  %c  %c sc:%d",
                           board[i][0] ? c[0] : '.',
                           board[i][1] ? c[1] : '.', 
                           board[i][2] ? c[2] : '.',
                           board[i][3] ? c[3] : '.', 
                           score);
        }
        else
        {
            // 安全地显示info信息
            const char* display_info = "";
            if(i-1 < 3 && info[i-1] != NULL) {
                display_info = info[i-1];
            }
            
            OLED_Printf_Line(i, "%c  %c  %c  %c %s   ",
                           board[i][0] ? c[0] : '.',
                           board[i][1] ? c[1] : '.',
                           board[i][2] ? c[2] : '.',
                           board[i][3] ? c[3] : '.',
                           display_info);
        }
    }
    OLED_Refresh_Dirty();
}

/**
 * @brief 游戏主运行函数
 * @details 实现游戏主循环，处理游戏逻辑、按键输入和体感控制
 */
void game_running_2048()
{
    init();
    init_info();  // 初始化info数组
    u8 key;

    static u8 mpu_initialized = 0;
    if(!mpu_initialized)
    {
        MPU_Init();
        mpu_initialized = 1;
    }

    while(1)
    {IWDG_ReloadCounter();
        if(isGameover())
        {
            flagisgameover = 1;
        }

        printBoard();

        int fx = getmove();
        if(fx != -1 && move(fx))
        {
            addNum();
            delay_ms(300);
        }

        key = KEY_Get();
        if(key)
        {
            switch(key)
            {
            case KEY2_PRES:
                cleanup_info();  // 清理内存
                OLED_Clear();
                return;
            default:
                break;
            }
        }
    }
}

/**
 * @brief 游戏菜单选项数组
 * @details 定义游戏主菜单的选项
 */
char *menu_2048_opt[] = {
    "start"};
u8 last_page_2048 = 99;
#define SHOWING_NUM 4
#define m_TOTAL_ITEMS (sizeof(menu_2048_opt) / sizeof(menu_2048_opt[0]))
/**
 * @brief 2048游戏菜单渲染函数
 * @param selected 当前选中的菜单项索引
 * @details 根据选中状态渲染菜单界面，支持分页显示
 */
void menu_2048_oled_RE(u8 selected)
{
  u8 page = selected / SHOWING_NUM;
  if (last_page_2048 != page)
  {
    OLED_Clear();
    last_page_2048 = page;
  }
  // 当前在第几页（从0开始）
  u8 start_idx = page * SHOWING_NUM;              // 本页第一个选项的索引
  u8 items_this_page = m_TOTAL_ITEMS - start_idx; // 本页实际有多少项
  if (items_this_page > SHOWING_NUM)
    items_this_page = SHOWING_NUM;

  for (u8 i = 0; i < items_this_page; i++)
  {
    u8 current_idx = start_idx + i; // 实际在总列表中的索引

    // 判断是否是当前选中项
    char arrow = (current_idx == selected) ? '>' : ' ';

    // 显示这一行（假设第0行开始显示菜单）
    OLED_Printf_Line(i, "%c %s", arrow, menu_2048_opt[current_idx]);
  }

  // 如果本页不足4行，下面几行可以清空或留空
  for (u8 i = items_this_page; i < SHOWING_NUM; i++)
  {
    OLED_Clear_Line(i); // 8个空格清行
  }

  OLED_Refresh_Dirty();
}
/**
 * @brief 处理菜单选项选择
 * @param selected 选中的菜单项索引
 * @details 根据用户选择进入相应的游戏功能
 */
void menu_2048_enter_select(u8 selected)
{
  switch (selected)
  {
  case 0:
    game_running_2048();
    break;

  default:
    break;
  }
}

/**
 * @brief 2048游戏菜单主函数
 * @details 显示游戏主菜单，处理按键输入，控制菜单导航
 */
void menu_2048_oled()
{
  u8 flag_Re = 1;
  u8 key;
  u8 selected = 0;
  while (1)
  {IWDG_ReloadCounter();
    if (flag_Re)
    {
      menu_2048_oled_RE(selected);
      flag_Re = 0;
    }

    delay_ms(10);
    key = KEY_Get();
    if (key)
    {
      switch (key)
      {
      case KEY3_PRES:
        flag_Re = 1;
        menu_2048_enter_select(selected);
        break;
      case KEY2_PRES:
        OLED_Clear();
        return;
      default:
        break;
      }
    }
  }
}
