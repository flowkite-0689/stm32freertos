# STM32 项目 FatFs 文件系统移植说明文档

## 1. 概述

本文档详细说明了在 STM32 项目中移植和配置 FatFs 文件系统的过程。FatFs 是一个为小型嵌入式系统设计的通用 FAT 文件系统模块，支持 FAT12、FAT16 和 FAT32 文件系统。

本项目中，FatFs 被移植到使用 W25Q128 Flash 存储器的 STM32F4 系列微控制器上。

## 2. 移植状态

目前文件系统已经成功移植，包括以下功能：

- FatFs R0.16 版本已集成到项目中
- 实现了针对 W25Q128 Flash 存储器的底层磁盘 I/O 接口
- 支持 FAT32 文件系统格式
- 支持基本的文件操作（创建、读取、写入、删除、重命名）
- 支持目录操作（创建、列出目录内容）

## 3. 硬件配置

### 3.1 存储设备

- **型号**: W25Q128
- **容量**: 16MB (128Mbit)
- **接口**: SPI
- **页大小**: 256 字节
- **扇区大小**: 4KB (4096 字节)

### 3.2 SPI 接口配置

- **SPI 模块**: SPI1
- **时钟频率**: 42MHz (系统时钟 84MHz / 2)
- **模式**: 主模式，双线全双工
- **极性**: CPOL=0 (空闲时 SCK 为低电平)
- **相位**: CPHA=1 (数据在第一个边沿采样)
- **片选引脚**: PB14

## 4. 软件架构

### 4.1 目录结构

```
User/
├── ff16/                 # FatFs 文件系统源代码
│   ├── ff.c             # FatFs 核心模块
│   ├── ff.h             # FatFs 核心头文件
│   ├── diskio.c         # 磁盘 I/O 接口实现
│   ├── diskio.h         # 磁盘 I/O 接口头文件
│   ├── ffconf.h         # FatFs 配置文件
│   ├── ffsystem.c       # 系统相关函数
│   └── ffunicode.c      # Unicode 支持函数
└── code/
    ├── spi.c            # SPI 接口驱动和 W25Q128 控制函数
    └── spi.h            # SPI 接口头文件
```

### 4.2 主要配置参数

在 `ff16/ffconf.h` 中的主要配置：

- `FF_FS_READONLY`: 0 (读写模式)
- `FF_FS_MINIMIZE`: 0 (启用所有基本功能)
- `FF_USE_MKFS`: 0 (禁用 f_mkfs 函数)
- `FF_USE_FASTSEEK`: 0 (禁用快速寻道功能)
- `FF_CODE_PAGE`: 936 (简体中文)
- `FF_USE_LFN`: 1 (启用长文件名支持)
- `FF_MAX_LFN`: 64 (最大长文件名长度)
- `FF_VOLUMES`: 1 (支持 1 个逻辑卷)
- `FF_MIN_SS`/`FF_MAX_SS`: 512 (扇区大小)
- `FF_FS_TINY`: 1 (启用精简缓冲区配置)
- `FF_FS_EXFAT`: 0 (不支持 exFAT)
- `FF_FS_NORTC`: 1 (无实时时钟，使用固定时间戳)

## 5. 磁盘 I/O 实现

### 5.1 物理驱动映射

在 `diskio.c` 中定义了物理驱动映射：
- DEV_FLASH (0): W25Q128 Flash 存储器
- DEV_MMC (1): MMC/SD 卡（未实现）
- DEV_USB (2): USB 存储设备（未实现）

### 5.2 核心函数实现

1. **disk_initialize**: 初始化 W25Q128，检查 JEDEC ID
2. **disk_status**: 返回驱动器状态
3. **disk_read**: 从 Flash 读取数据
4. **disk_write**: 向 Flash 写入数据（包含扇区擦除）
5. **disk_ioctl**: 控制函数，支持获取扇区数量、扇区大小等

## 6. W25Q128 驱动实现

### 6.1 主要功能函数

- `SPI1_Init`: 初始化 SPI1 接口
- `W25Q128_ReadID`: 读取芯片 ID
- `W25Q128_SectorErase`: 扇区擦除
- `W25Q128_WritePage`: 页编程
- `W25Q128_BufferWrite`: 缓冲区写入（支持跨页）
- `W25Q128_ReadData`: 数据读取

### 6.2 特殊处理

- 写入前自动执行扇区擦除
- 自动处理跨页写入
- 地址边界检查

## 7. 使用示例

### 7.1 初始化和挂载

```c
// 初始化 SPI 和 W25Q128
SPI1_Init();

// 挂载文件系统
FATFS fs;
FRESULT res = f_mount(&fs, "0:", 1);
if (res != FR_OK) {
    // 处理挂载失败
}
```

### 7.2 文件操作

```c
// 创建并写入文件
FIL file;
FRESULT res = f_open(&file, "0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
if (res == FR_OK) {
    const char *text = "Hello World!\r\n";
    UINT bw;
    f_write(&file, text, strlen(text), &bw);
    f_close(&file);
}

// 读取文件
res = f_open(&file, "0:/test.txt", FA_READ);
if (res == FR_OK) {
    char buffer[128];
    UINT br;
    f_read(&file, buffer, sizeof(buffer), &br);
    f_close(&file);
    // 处理读取的数据
}
```

### 7.3 目录操作

```c
// 列出目录内容
DIR dir;
FILINFO fno;
res = f_opendir(&dir, "0:/");
if (res == FR_OK) {
    while (1) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) break;
        if (fno.fname[0] == '.') continue;
        
        const char *type = (fno.fattrib & AM_DIR) ? "[DIR]" : "[FILE]";
        printf("  %-6s %-12s %lu bytes\r\n", type, fno.fname, fno.fsize);
    }
    f_closedir(&dir);
}
```

## 8. 测试验证

### 8.1 已验证功能

- 文件系统挂载和格式化
- 文件创建、写入、读取
- 文件追加写入
- 文件删除和重命名
- 目录创建和列表
- 磁盘空间信息查询

### 8.2 性能特点

- 读取速度：取决于 SPI 时钟频率
- 写入速度：受 Flash 擦除/编程时间限制
- 支持标准 POSIX 文件操作接口

## 9. 注意事项

1. W25Q128 是 NOR Flash，写入前必须先擦除扇区
2. 扇区大小为 4KB，擦除操作以扇区为单位
3. 页编程大小最大为 256 字节
4. 文件系统使用固定时间戳（因为未配置 RTC）
5. 不支持 exFAT，仅支持 FAT12/FAT16/FAT32

## 10. 可能的改进方向

1. 启用 `FF_USE_MKFS` 以支持运行时格式化
2. 添加 RTC 支持以启用真实时间戳
3. 实现多卷支持（如同时支持 SD 卡）
4. 启用 exFAT 支持以处理大文件
5. 添加 wear leveling（磨损均衡）算法以延长 Flash 寿命