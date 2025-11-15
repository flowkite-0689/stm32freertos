#include "music.h"
#include "timer_general.h"
#include "delay.h"  // 需要包含延时函数头文件
#include <stdlib.h>  // 需要包含malloc和free
#include "key.h"

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
    // 使用静态内存分配，避免malloc失败
    static Note converted_melody[200]; // 静态数组，足够大的大小
    
    if(length > 200) {
        printf("Melody too long, truncating to 200 notes\n");
        length = 200;
    }
    
    printf("Playing melody with %d notes (static allocation)\n", length);
    convert_numbered_to_note(numbered_melody, converted_melody, length, time_sig, bpm);
    Play_Melody(converted_melody, length);
}

//-------------------------------------------------------------------




//-------------------------------------------------------------------



//-------------------------------------------------------------------





