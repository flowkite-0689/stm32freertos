# OLED增强模拟器

这是一个基于SDL2的OLED模拟器，完全兼容您在真实OLED屏幕上使用的所有函数。

## 功能特点

- 完整实现了OLED.c中的所有显示函数
- 集成了OLED_Print.c中的所有打印功能
- 支持128x64分辨率的OLED显示
- 提供SDL2图形界面，可直接在PC上运行
- 完整的图形绘制、文字显示、图片显示等功能

## 编译和运行

### 安装依赖

在Linux上：
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev cmake

# CentOS/Fedora
sudo yum install SDL2-devel cmake

# Arch Linux
sudo pacman -S sdl2 cmake
```

在macOS上：
```bash
brew install sdl2 cmake
```

在Windows上：
1. 下载SDL2开发库：https://www.libsdl.org/download-2.0.php
2. 下载CMake：https://cmake.org/download/
3. 解压并将include和lib目录添加到项目路径

### 使用CMake编译（推荐）

方法1：标准CMake
```bash
mkdir build
cd build
cmake ..
make
```

方法2：使用高级CMake配置
```bash
mkdir build
cd build
cmake -f ../CMakeLists_advanced.txt ..
make
```

方法3：带选项的CMake
```bash
mkdir build
cd build
cmake -DBUILD_TESTS=ON -DENABLE_SIMULATOR_LOG=ON ..
make
```

### 运行

使用CMake：
```bash
make run
```

或者直接运行：
```bash
./build/bin/OLED_Enhanced_Simulator
```

### 其他构建目标

```bash
# 格式化代码
make format

# 静态分析（需要cppcheck）
make analyze

# 无头模式运行（用于调试）
make debug-run

# 完全清理
make distclean

# 创建安装包
make package
```

## 支持的函数

### 基础显示函数

- `OLED_Init()` - 初始化OLED
- `OLED_Clear()` - 清屏
- `OLED_Refresh()` - 刷新显示
- `OLED_Refresh_Dirty()` - 局部刷新脏区域
- `OLED_Set_Dirty_Area()` - 设置脏区域

### 绘图函数

- `OLED_DrawPoint()` - 画点
- `OLED_DrawLine()` - 画线
- `OLED_DrawCircle()` - 画圆
- `OLED_ShowPicture()` - 显示图片

### 文字显示函数

- `OLED_ShowChar()` - 显示字符
- `OLED_ShowString()` - 显示字符串
- `OLED_ShowNum()` - 显示数字
- `OLED_ShowChinese()` - 显示中文

### 打印功能（来自OLED_Print.c）

- `OLED_Printf()` - 在指定位置格式化打印
- `OLED_Printf_Line()` - 在指定行打印
- `OLED_Clear_Line()` - 清除指定行
- `OLED_Display_Info()` - 显示系统信息
- `OLED_Display_Sensor()` - 显示传感器数据

### 高级功能

- `OLED_ColorTurn()` - 颜色翻转（正/反显示）
- `OLED_DisplayTurn()` - 屏幕翻转180度
- `OLED_DisPlay_On()` - 开启显示
- `OLED_DisPlay_Off()` - 关闭显示
- `OLED_ScrollDisplay()` - 滚动显示

## 使用示例

```c
// 初始化OLED
OLED_Init();

// 清屏
OLED_Clear();

// 显示文字
OLED_ShowString(0, 0, "Hello OLED!", 16, 1);

// 显示数字
OLED_ShowNum(0, 20, 12345, 5, 16, 1);

// 显示中文
OLED_ShowChinese(0, 40, 0, 16, 1); // 显示第一个汉字

// 使用Printf功能
OLED_Printf_Line(1, "系统时间: %dms", SDL_GetTicks());

// 显示传感器数据
OLED_Display_Sensor("温度", 25.6, 18.2, "°C");

// 刷新显示（模拟器中由SDL自动处理）
OLED_Refresh_Dirty();
```

## 注意事项

1. 模拟器中的`OLED_GRAM`数组结构与真实OLED一致，确保显示效果一致
2. 所有打印输出到控制台的函数都会显示操作日志
3. 按X键或关闭窗口可退出模拟器
4. 模拟器使用SDL2实现，确保已正确安装SDL2开发库

## 文件结构

- `oled_simulator_enhanced.c` - 增强版模拟器主文件
- `CMakeLists.txt` - 基础CMake构建脚本
- `CMakeLists_advanced.txt` - 高级CMake构建脚本（更多选项）
- `Makefile` - 传统Make构建脚本
- `config.h.in` - CMake配置文件模板
- `README.md` - 使用说明
- `../logo.h` - Logo图片数据
- `../../oledfont.h` - 字体数据

## 与真实硬件的差异

1. 模拟器中的`delay_ms()`函数使用SDL_Delay实现
2. I2C通信通过打印日志模拟，不进行真实通信
3. 显示刷新由SDL渲染系统处理，无需手动刷新显存到硬件

## 常见问题

1. **编译错误：找不到SDL2**
   - 确保已正确安装SDL2开发库
   - 检查CMake是否正确找到SDL2：`cmake --debug-output ..`

2. **运行时窗口无法打开**
   - 检查图形环境是否正常
   - 尝试在终端直接运行查看错误信息

3. **显示内容不正确**
   - 检查字体数据是否正确包含
   - 确认图片数据格式是否正确

## CMake选项详解

- `BUILD_TESTS=ON` - 启用测试（需要添加测试目录）
- `ENABLE_SIMULATOR_LOG=ON/OFF` - 启用/禁用模拟器日志输出
- `USE_SYSTEM_SDL2=ON/OFF` - 使用系统SDL2或自定义SDL2

## CMake高级功能

使用`CMakeLists_advanced.txt`可以享受以下额外功能：
- 版本信息集成
- 代码格式化（需要clang-format）
- 静态分析（需要cppcheck）
- 包管理（支持创建DEB/RPM/DMG等包）
- 无头模式运行（适合CI/CD环境）
- 详细的配置信息输出