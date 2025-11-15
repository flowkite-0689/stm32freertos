# 蜂鸣器乐谱播放功能说明

## 功能概述

本功能通过PWM控制蜂鸣器频率来播放不同音调，实现音乐播放功能。

## 核心功能

### 1. 音符定义

系统预定义了常用音符频率（单位：Hz）：

```c
#define NOTE_C4  262  // 中央C
#define NOTE_D4  294  // D
#define NOTE_E4  330  // E
#define NOTE_F4  349  // F
#define NOTE_G4  392  // G
#define NOTE_A4  440  // A
#define NOTE_B4  494  // B
#define NOTE_C5  523  // 高音C
// ... 更多音符
```

### 2. 音符结构体

```c
typedef struct {
    uint16_t frequency;  // 频率（Hz）
    uint16_t duration;   // 持续时间（ms）
} Note;
```

### 3. 主要API

#### Set_PWM_Frequency(uint32_t frequency)
- **功能**：设置PWM频率产生指定音调
- **参数**：frequency - 频率值（Hz），0表示静音
- **示例**：
  ```c
  Set_PWM_Frequency(NOTE_C4);  // 播放中央C
  Set_PWM_Frequency(0);         // 静音
  ```

#### Play_Note(uint16_t frequency, uint16_t duration)
- **功能**：播放单个音符
- **参数**：
  - frequency - 频率（Hz）
  - duration - 持续时间（ms）
- **示例**：
  ```c
  Play_Note(NOTE_E4, 500);  // 播放E音，持续500ms
  ```

#### Play_Melody(Note *melody, uint16_t length)
- **功能**：播放完整旋律
- **参数**：
  - melody - 音符数组
  - length - 数组长度
- **示例**：
  ```c
  Note song[] = {{NOTE_C4, 500}, {NOTE_D4, 500}};
  Play_Melody(song, sizeof(song)/sizeof(Note));
  ```

## 使用方法

### 1. 初始化
```c
TIM13_PWM_Init();  // 初始化PWM
```

### 2. 定义歌曲
```c
Note my_song[] = {
    {NOTE_C4, 500},  // 第一拍，中央C
    {NOTE_D4, 500),  // 第二拍，D音
    {NOTE_REST, 500}  // 休止符
};
```

### 3. 播放歌曲
```c
Play_Melody(my_song, sizeof(my_song)/sizeof(Note));
```

## 示例程序

系统中已包含《小星星》完整示例，按键KEY3即可播放。

## 注意事项

1. 播放时会阻塞程序，如需非阻塞播放需使用定时器中断
2. 持续时间单位为毫秒，可根据节奏调整
3. 频率范围建议在100Hz-5000Hz之间以获得最佳音效
4. 播放结束会自动静音