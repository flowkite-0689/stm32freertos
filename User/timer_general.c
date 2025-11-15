#include "timer_general.h"
#include "delay.h"  // 需要包含延时函数头文件
#include <stdlib.h>  // 需要包含malloc和free

void TIM13_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
    
    // 2. 配置GPIOF Pin9为复用功能
    GPIO_InitStructure.GPIO_Pin = BEEP0_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    //2222配置蜂鸣器'
     GPIO_InitStructure.GPIO_Pin = BEEP0_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(BEEP0_PORT, &GPIO_InitStructure);
    
    // 3. 将PF9连接到TIM13（复用功能映射）
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource8, GPIO_AF_TIM13);
    
    // 4. 配置TIM13时基单元(TIM4内部时钟源默认84M)
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;           	// 预分频器84-1
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseStructure.TIM_Period = 999;             		// 自动重装载值1000-1
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 时钟分频(死区和输入采样时才有用）
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数次数，高级定时器才有
    TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);
    
    // 5. 配置TIM13通道1为PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;   	// PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;// 互补功能,只有高级定时器才有
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;    // 输出极性高
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;	// 定时器处于空闲状态（非工作状态）时，输出通道的电平状态,只有高级定时器才有
    TIM_OCInitStructure.TIM_Pulse = 0;     			// 比较寄存器数值
    TIM_OC1Init(TIM13, &TIM_OCInitStructure);		// 配置输出模式
	
	//在更新事件发生时才将新的比较值从预装载寄存器传输到活动寄存器。
    TIM_OC1PreloadConfig(TIM13, TIM_OCPreload_Enable);  // 使能预装载
    
    // 6. 使能TIM13预分频器自动重装载
	// 在更新事件发生时才将新的ARR值从预装载寄存器传输到活动寄存器。
    TIM_ARRPreloadConfig(TIM13, ENABLE);
    
    // 7. 启动TIM13
    TIM_Cmd(TIM13, ENABLE);
}
#include "htim.h"

// 设置PWM占空比百分比 (0-100)
void Set_PWM_Percentage(uint8_t percentage)
{
    if(percentage > 100)
        percentage = 100;
    
    uint16_t duty_cycle = (uint16_t)((percentage * 1000) / 100);
    if(duty_cycle > 1000)
        duty_cycle = 1000;
    TIM_SetCompare1(TIM13, duty_cycle);  // 设置比较寄存器数值
}

// 设置PWM频率以产生不同音调
void Set_PWM_Frequency(uint32_t frequency)
{
    uint32_t prescaler;
    uint32_t period;
    
    // TIM13时钟为84MHz
    if(frequency == 0) {
        TIM_Cmd(TIM13, DISABLE);  // 停止PWM输出
        return;
    }
    
    // 计算分频和周期，使频率尽可能精确
    prescaler = 84 - 1;  // 84MHz/84 = 1MHz
    period = 1000000 / frequency - 1;  // 1MHz/frequency - 1
    
    // 确保周期在合理范围内
    if(period < 1) period = 1;
    if(period > 65535) period = 65535;
    
    TIM_SetAutoreload(TIM13, period);
    TIM_PrescalerConfig(TIM13, prescaler, TIM_PSCReloadMode_Immediate);
    
    // 设置50%占空比以获得最佳音效
    TIM_SetCompare1(TIM13, period / 2);
    
    TIM_Cmd(TIM13, ENABLE);  // 启动PWM输出
}

// 播放单个音符
void Play_Note(uint16_t frequency, uint16_t duration)
{
    if(frequency == NOTE_REST) {
        Set_PWM_Frequency(0);  // 停止发声
    } else {
        Set_PWM_Frequency(frequency);  // 设置频率
    }
    delay_ms(duration);  // 持续指定时间
}

// 播放旋律
void Play_Melody(Note *melody, uint16_t length)
{
    uint16_t i;
    for(i = 0; i < length; i++) {
        Play_Note(melody[i].frequency, melody[i].duration);
    }
    Set_PWM_Frequency(0);  // 结束后停止发声
}

// 获取拍号信息
TimeSignatureInfo get_time_signature_info(TimeSignature time_sig)
{
    TimeSignatureInfo info;
    
    switch(time_sig) {
        case TIME_SIGNATURE_4_4:
            info.beats_per_measure = 4;
            info.beat_note_value = 4;
            info.base_note_value = 4;  // 基本单位为四分音符
            break;
        case TIME_SIGNATURE_6_8:
            info.beats_per_measure = 6;
            info.beat_note_value = 8;
            info.base_note_value = 8;  // 基本单位为八分音符
            break;
        case TIME_SIGNATURE_3_4:
            info.beats_per_measure = 3;
            info.beat_note_value = 4;
            info.base_note_value = 4;  // 基本单位为四分音符
            break;
        default:
            info.beats_per_measure = 4;
            info.beat_note_value = 4;
            info.base_note_value = 4;
            break;
    }
    
    return info;
}

// 增强的频率获取函数，支持升降号
uint16_t get_frequency_from_numbered_note(const NumberedNote *note)
{
    if(note->note == 0) return NOTE_REST;  // 休止符
    if(note->note < 1 || note->note > 7) return NOTE_REST;  // 无效音符
    
    uint16_t base_freq = 0;
    
    // 先获取基础频率
    switch(note->note) {
        case 1:  // Do
            if(note->octave == 3) base_freq = NOTE_C3;
            else if(note->octave == 4) base_freq = NOTE_C4;
            else if(note->octave == 5) base_freq = NOTE_C5;
            break;
        case 2:  // Re
            if(note->octave == 3) base_freq = NOTE_D3;
            else if(note->octave == 4) base_freq = NOTE_D4;
            else if(note->octave == 5) base_freq = NOTE_D5;
            break;
        case 3:  // Mi
            if(note->octave == 3) base_freq = NOTE_E3;
            else if(note->octave == 4) base_freq = NOTE_E4;
            else if(note->octave == 5) base_freq = NOTE_E5;
            break;
        case 4:  // Fa
            if(note->octave == 3) base_freq = NOTE_F3;
            else if(note->octave == 4) base_freq = NOTE_F4;
            else if(note->octave == 5) base_freq = NOTE_F5;
            break;
        case 5:  // Sol
            if(note->octave == 3) base_freq = NOTE_G3;
            else if(note->octave == 4) base_freq = NOTE_G4;
            else if(note->octave == 5) base_freq = NOTE_G5;
            break;
        case 6:  // La
            if(note->octave == 3) base_freq = NOTE_A3;
            else if(note->octave == 4) base_freq = NOTE_A4;
            else if(note->octave == 5) base_freq = NOTE_A5;
            break;
        case 7:  // Si
            if(note->octave == 3) base_freq = NOTE_B3;
            else if(note->octave == 4) base_freq = NOTE_B4;
            else if(note->octave == 5) base_freq = NOTE_B5;
            break;
    }
    
    // 处理升降号
    if(note->accidental == 1) {  // 升号
        // 增加约6%的频率（半音）
        base_freq = (uint16_t)(base_freq * 1.059463);
    } else if(note->accidental == 2) {  // 降号
        // 减少约6%的频率（半音）
        base_freq = (uint16_t)(base_freq / 1.059463);
    }
    
    return base_freq;
}

// 修正后的时长获取函数，根据正确的时值表示规则
uint16_t get_duration_from_note_value(const NumberedNote *note, TimeSignature time_sig, uint8_t bpm)
{
    TimeSignatureInfo info = get_time_signature_info(time_sig);
    
    // 计算基本单位时长（ms）
    uint16_t base_duration = (uint16_t)(60000.0 / bpm / (info.base_note_value / 4.0));
    
    // 根据下划线数量确定基础音符时值
    uint16_t duration;
    switch(note->underline_count) {
        case 0:  // 无下划线 = 四分音符
            duration = base_duration * 2;
            break;
        case 1:  // 一条下划线 = 八分音符
            duration = base_duration;
            break;
        case 2:  // 两条下划线 = 十六分音符
            duration = base_duration / 2;
            break;
        default:
            duration = base_duration * 2;  // 默认为四分音符
            break;
    }
    
    // 处理延音：每个延音延长基础音符时值
    for(uint8_t i = 0; i < note->extension_count; i++) {
        duration += duration;  // 每次翻倍
    }
    
    // 处理附点（增加原时值的一半）
    if(note->dotted) {
        duration += duration / 2;
    }
    
    return duration;
}

// 增强的转换函数
void convert_numbered_to_note(const NumberedNote *numbered_melody, Note *melody, uint16_t length, TimeSignature time_sig, uint8_t bpm)
{
    for(uint16_t i = 0; i < length; i++) {
        melody[i].frequency = get_frequency_from_numbered_note(&numbered_melody[i]);
        melody[i].duration = get_duration_from_note_value(&numbered_melody[i], time_sig, bpm);
    }
}

// 增强的播放函数
void Play_Numbered_Melody(const NumberedNote *numbered_melody, uint16_t length, TimeSignature time_sig, uint8_t bpm)
{
    Note *converted_melody = (Note*)malloc(length * sizeof(Note));
    if(converted_melody == NULL) return;
    
    convert_numbered_to_note(numbered_melody, converted_melody, length, time_sig, bpm);
    Play_Melody(converted_melody, length);
    
    free(converted_melody);
}
