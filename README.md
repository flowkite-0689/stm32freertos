# STM32 FreeRTOS 智能手表项目

这是一个基于STM32F4xx微控制器和FreeRTOS实时操作系统的智能手表项目。

## 项目概述

该项目实现了一个功能丰富的智能手表系统，采用FreeRTOS进行多任务管理：
- **FreeRTOS实时操作系统**：提供多任务调度、内存管理、定时器等核心功能
- **OLED显示屏**：提供用户界面显示功能
- **多功能模块**：包含计步器、温湿度监测、游戏、文件系统等功能
- **传感器集成**：MPU6050加速度传感器、DHT11温湿度传感器等
- **丰富外设**：蜂鸣器、按键、LED、RTC时钟等

## 硬件要求

- STM32F4xx系列开发板
- OLED显示屏（I2C接口）
- MPU6050加速度传感器
- DHT11温湿度传感器
- 蜂鸣器
- 多个按键
- LEDs

## 功能特性

- **FreeRTOS实时操作系统**：多任务调度和管理
  - 任务创建、删除、挂起和恢复
  - 队列、信号量、互斥锁等IPC机制
  - 软件定时器和事件组
- **OLED显示系统**：
  - 128x64分辨率显示
  - 中英文显示支持
  - UI菜单系统
  - 游戏界面（2048游戏等）
- **传感器模块**：
  - MPU6050：加速度和陀螺仪数据采集
  - DHT11：温湿度数据读取
  - 计步器功能
- **外设驱动**：
  - 按键扫描和处理
  - LED控制
  - 蜂鸣器音乐播放
  - UART通信（DMA模式）
- **应用功能**：
  - 实时时钟（RTC）
  - 秒表功能
  - 闹钟系统
  - 手电筒功能
  - 文件系统测试
  - 看门狗测试
  - RFID读取功能
- **用户界面**：
  - 统一菜单架构
  - 设置界面
  - 多功能集成界面

## 文件结构

```
├── User/
│   ├── main.c                    # 主程序文件
│   ├── stm32f4xx_it.c            # 中断服务程序
│   ├── stm32f4xx_it.h            # 中断头文件
│   ├── FreeRTOS/                 # FreeRTOS内核源码
│   │   ├── include/              # FreeRTOS头文件
│   │   ├── portable/             # 移植层代码
│   │   └── *.c                   # FreeRTOS内核文件
│   ├── code/                     # 基础驱动模块
│   │   ├── gpio.c/.h             # GPIO驱动
│   │   ├── uart_dma.c/.h         # UART DMA通信
│   │   ├── key.c/.h              # 按键驱动
│   │   ├── led.c/.h              # LED控制
│   │   ├── beep.c/.h             # 蜂鸣器驱动
│   │   ├── rtc_date.c/.h         # RTC时钟
│   │   └── ...                   # 其他基础驱动
│   ├── OLED/                     # OLED显示模块
│   │   ├── oled.c/.h             # OLED基础驱动
│   │   ├── oled_print.c/.h       # OLED打印函数
│   │   └── simulator/            # OLED模拟器
│   ├── ui/                       # 用户界面模块
│   │   ├── ui.h                  # UI模块头文件
│   │   ├── unified_menu.c/.h     # 统一菜单架构
│   │   ├── 2048_oled.c/.h        # 2048游戏
│   │   ├── stopwatch.c/.h        # 秒表功能
│   │   ├── alarm_*.c/.h          # 闹钟功能
│   │   └── ...                   # 其他UI功能
│   ├── MPU6050/                  # MPU6050传感器模块
│   │   ├── MPU6050.c/.h          # MPU6050驱动
│   │   ├── simple_pedometer.c/.h # 简单计步器
│   │   └── eMPL/                 # 运动处理库
│   ├── ff16/                     # FatFs文件系统
│   └── rfid/                     # RFID模块
├── Libraries/
│   ├── CMSIS/                    # ARM CMSIS库
│   └── STM32F4xx_StdPeriph_Driver/  # STM32F4xx标准外设库
├── Output/                       # 编译输出目录
└── Project/                      # 项目配置文件
```

## 编译和烧录

1. 使用Keil MDK或其他兼容IDE打开项目
2. 编译项目生成可执行文件
3. 通过ST-Link、J-Link等调试器烧录到开发板

## 主要代码说明

### FreeRTOS任务创建
```c
// 创建主任务
xTaskCreate(app_main_task, "MainTask", 512, NULL, 2, &app_main_task_handle);

// 创建应用任务
xTaskCreate(app_task1, "Task1", 256, NULL, 1, &app_task1_handle);
```

### OLED显示初始化
```c
void OLED_Init(void)
{
    // I2C初始化
    Soft_I2C_Init();
    
    // OLED初始化序列
    OLED_Write_Command(0xAE); // 关闭显示
    // ... 其他初始化命令
}
```

### MPU6050传感器初始化
```c
void MPU6050_Init(void)
{
    // 初始化MPU6050
    mpu_init();
    
    // 配置DMP
    dmp_load_motion_driver_firmware();
    // ... 其他配置
}
```

### 统一菜单架构
```c
typedef struct {
    const char *name;           // 菜单项名称
    void (*handler)(void);      // 处理函数
    uint8_t sub_menu_count;     // 子菜单数量
    struct menu_item *sub_menu; // 子菜单指针
} menu_item_t;
```

## 开发环境

- IDE：Keil MDK-ARM
- 芯片：STM32F4xx系列
- 操作系统：FreeRTOS v10.0+
- 库：STM32F4xx标准外设库 + CMSIS
- 文件系统：FatFs
- 显示：OLED 128x64 (SSD1306)
- 传感器：MPU6050, DHT11

## 项目特色

1. **多任务架构**：基于FreeRTOS的多任务管理，提供实时响应能力
2. **模块化设计**：各功能模块独立封装，便于维护和扩展
3. **丰富功能**：集成多种传感器和应用功能，展现智能手表核心特性
4. **统一菜单**：设计了一套通用的菜单架构，便于快速开发新功能
5. **OLED模拟器**：提供PC端OLED显示模拟，便于调试UI界面

## 扩展方向

- 添加蓝牙通信功能
- 集成更多传感器（心率、血氧等）
- 优化功耗管理
- 添加更多应用功能
- 实现数据同步和云端存储

## 许可证

本项目仅供学习和参考使用。