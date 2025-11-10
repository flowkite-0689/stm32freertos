# STM32 LED Blink Project

这是一个基于STM32F4xx微控制器的LED闪烁示例项目。

## 项目概述

该项目实现了一个简单的LED闪烁功能：
- PF9引脚上的LED以1Hz频率闪烁（亮1秒，灭1秒）
- PF10引脚保持低电平状态
- 使用SysTick定时器实现精确延时

## 硬件要求

- STM32F4xx系列开发板
- LED连接到PF9和PF10引脚（或相应开发板上的用户LED）

## 功能特性

- **SysTick定时器**：配置为1ms中断，提供精确时基
- **GPIO配置**：PF9和PF10配置为推挽输出模式
- **延时函数**：基于SysTick的非阻塞延时实现
- **主循环**：实现LED的周期性闪烁

## 文件结构

```
├── User/
│   ├── main.c              # 主程序文件
│   ├── stm32f4xx_it.c      # 中断服务程序
│   └── stm32f4xx_it.h      # 中断头文件
├── Libraries/
│   ├── CMSIS/              # ARM CMSIS库
│   └── STM32F4xx_StdPeriph_Driver/  # STM32F4xx标准外设库
├── Output/                 # 编译输出目录
└── Project/                # 项目配置文件
```

## 编译和烧录

1. 使用Keil MDK或其他兼容IDE打开项目
2. 编译项目生成可执行文件
3. 通过ST-Link、J-Link等调试器烧录到开发板

## 主要代码说明

### SysTick初始化
```c
void Systick_Init(void)
{
    // 配置为1ms中断
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1);  // 配置失败则在此循环
    }
    // 设置最高中断优先级
    NVIC_SetPriority(SysTick_IRQn, 0);
}
```

### GPIO配置
```c
void GPIO_Init_LED(void)
{
    // 使能GPIOF时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    
    // 配置PF9和PF10为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    GPIO_Init(GPIOF, &GPIO_InitStructure);
}
```

## 开发环境

- IDE：Keil MDK-ARM
- 芯片：STM32F4xx系列
- 库：STM32F4xx标准外设库 + CMSIS

## 许可证

本项目仅供学习和参考使用。