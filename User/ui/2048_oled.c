#include "2048_oled.h"

#define SIZE 4

int board[SIZE][SIZE];
int score = 0;
int flagisgameover=0;
// 新增数字的逻辑
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

// 初始化逻辑
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

// 数字移动逻辑
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

// 游戏结束的逻辑
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

// 靠mpu6050的体感来操控方向的方法
int getmove()
{
    short ax, ay, az;
    static int last_direction = -1;
    int current_direction = -1;
    int threshold = 3000; // 加速度阈值，可根据实际情况调整
    
    // 获取加速度数据
    if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0)
    {
        // 根据加速度判断倾斜方向
        // 注意：这里的阈值可能需要根据实际情况调整
        if (abs(ax) < 1000 && abs(ay) < 1000) {
            // 接近水平，不移动
            return -1;
        }
        
        // 判断主要倾斜方向
        if (abs(ax) > abs(ay)) {
            // X轴方向占主导
            if (ax > threshold) {
                current_direction = 1; // 右
            } else if (ax < -threshold) {
                current_direction = 0; // 左
            }
        } else {
            // Y轴方向占主导
            if (ay > threshold) {
                current_direction = 2; // 下
            } else if (ay < -threshold) {
                current_direction = 3; // 上
            }
        }
        
        // 只有在方向发生改变时才返回新的方向，避免连续移动
        if (current_direction != last_direction && current_direction != -1) {
            last_direction = current_direction;
            return current_direction;
        }
        
        // 如果回到水平状态，重置last_direction
        if (abs(ax) < 1000 && abs(ay) < 1000) {
            last_direction = -1;
        }
    }
    
    return -1; // 无有效移动
}


//游戏运行界面

//打印棋盘
void printBoard()
{
  for (int i = 0; i < SIZE; i++)
  {
    char c[SIZE];
    for (int  j = 0; j < SIZE; j++)
    {
      int  number = board[i][j];
      if (number<10)
      {
        c[j]=number+'0';
      }else if (number == 16  )
      {
        c[j] = 'A';
      }else if (number == 32 )
      {
        c[j] = 'B';
      }else if (number == 64)
      {
        c[j] = 'C';
      }else if (number == 128)
      {
        c[j] = 'D';
      }else if (number == 256)
      {
        c[j] = 'E';
      }else if (number == 512)
      {
        c[j] = 'F';
      }else if (number == 1024)
      {
        c[j] = 'G';
      }else if (number == 2048)
      {
        c[j] = 'H';
      }
      
      
      
      
      
    }
    
    if (i==0)
    {
        OLED_Printf_Line(i,"%c  %c  %c  %c score:%d",
      board[i][0]?c[0]:'.',
    board[i][1]?c[1]:'.',
  board[i][2]?c[2]:'.',
board[i][3]?c[3]:'.',score);
    }
    else
    {
    OLED_Printf_Line(i,"%c  %c  %c  %c %s",
     board[i][0]?c[0]:'.',
    board[i][1]?c[1]:'.',
  board[i][2]?c[2]:'.',
board[i][3]?c[3]:'.',
flagisgameover?" OVER":" ");
  }}
  OLED_Refresh_Dirty();
}




//
void game_running_2048()
{
 init();
 u8 key ;
 
 // 确保MPU6050已初始化
 static u8 mpu_initialized = 0;
 if (!mpu_initialized) {
     MPU_Init();
     mpu_initialized = 1;
 }
 
 while (1)
 {
  if (isGameover())
  {
    flagisgameover=1;
  }
  
  printBoard();

  int fx = getmove(); // 获得移动方向

  if (fx != -1 && move(fx)) // 只有当有有效移动方向且实际移动时才添加新数字
  {
    addNum();
    delay_ms(300); // 添加延迟防止过于灵敏的响应
  }
  
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
  
 }
}


// 游戏菜单界面
// 选项
char *menu_2048_opt[] = {
    "start"};
u8 last_page_2048 = 99;
#define SHOWING_NUM 4
#define m_TOTAL_ITEMS (sizeof(menu_2048_opt) / sizeof(menu_2048_opt[0]))
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
// 进入到所选
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

//游戏菜单主函数
void menu_2048_oled()
{
  u8 flag_Re = 1;
  u8 key;
  u8 selected = 0;
  while (1)
  {
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

