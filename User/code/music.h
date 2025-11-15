#ifndef MUSIC_H
#define MUSIC_H
#include "stm32f4xx.h"

// 音符频率定义
#define NOTE_REST 0
#define NOTE_C3  131   // 低音C (1̲)
#define NOTE_D3  147   // 低音D (2̲)
#define NOTE_E3  165   // 低音E (3̲)
#define NOTE_F3  175   // 低音F (4̲)
#define NOTE_G3  196   // 低音G (5̲)
#define NOTE_A3  220   // 低音A (6̲)
#define NOTE_B3  247   // 低音B (7̲)

#define NOTE_C4  262   // 中音C (1)
#define NOTE_D4  294   // 中音D (2)
#define NOTE_E4  330   // 中音E (3)
#define NOTE_F4  349   // 中音F (4)
#define NOTE_G4  392   // 中音G (5)
#define NOTE_A4  440   // 中音A (6)
#define NOTE_B4  494   // 中音B (7)

#define NOTE_C5  523   // 高音C (1̇)
#define NOTE_D5  587   // 高音D (2̇)
#define NOTE_E5  659   // 高音E (3̇)
#define NOTE_F5  698   // 高音F (4̇)
#define NOTE_G5  784   // 高音G (5̇)
#define NOTE_A5  880   // 高音A (6̇)
#define NOTE_B5  988   // 高音B (7̇)

#define NOTE_C6  1047  // 更高音C (1̇̇)

// 升降调定义
#define NOTE_CS3 139   // 低音升C (1̲#)
#define NOTE_DS3 156   // 低音升D (2̲#)
#define NOTE_FS3 185   // 低音升F (4̲#)
#define NOTE_GS3 208   // 低音升G (5̲#)
#define NOTE_AS3 233   // 低音升A (6̲#)

#define NOTE_CS4 277   // 中音升C (1#)
#define NOTE_DS4 311   // 中音升D (2#)
#define NOTE_FS4 370   // 中音升F (4#)
#define NOTE_GS4 415   // 中音升G (5#)
#define NOTE_AS4 466   // 中音升A (6#)

#define NOTE_CS5 554   // 高音升C (1̇#)
#define NOTE_DS5 622   // 高音升D (2̇#)
#define NOTE_FS5 740   // 高音升F (4̇#)
#define NOTE_GS5 831   // 高音升G (5̇#)
#define NOTE_AS5 932   // 高音升A (6̇#)

// 时值系统 - 支持不同拍号
typedef enum {
    TIME_SIGNATURE_4_4 = 0,  // 4/4拍，基本单位为四分音符
    TIME_SIGNATURE_6_8 = 1,  // 6/8拍，基本单位为八分音符
    TIME_SIGNATURE_3_4 = 2   // 3/4拍，基本单位为四分音符
} TimeSignature;

// 音符结构体
typedef struct {
    uint16_t frequency;  // 频率(Hz)
    uint16_t duration;   // 持续时间(ms)
} Note;

// 增强的数字简谱结构体
typedef struct {
    uint8_t note;          // 音符(1-7, 0表示休止符)
    uint8_t octave;        // 八度(3=低音L, 4=中音, 5=高音h)
    uint8_t accidental;    // 升降号(0=无, 1=升号#, 2=降号b)
    uint8_t underline_count;// 下划线数量(0=四分, 1=八分, 2=十六)
    uint8_t extension_count;// 延音数量(0-2)，每个延音延长基础音符时值
    uint8_t dotted;        // 是否为附点音符(0=否, 1=是)
} NumberedNote;

// 拍号信息结构体
typedef struct {
    uint8_t beats_per_measure;    // 每小节拍数
    uint8_t beat_note_value;      // 拍音符时值(4=四分音符, 8=八分音符)
    uint8_t base_note_value;      // 基本音符时值(用于计算)
} TimeSignatureInfo;

void Play_Melody(Note *melody, uint16_t length);
void Play_Note(uint16_t frequency, uint16_t duration);

// 增强的数字简谱函数
uint16_t get_frequency_from_numbered_note(const NumberedNote *note);
uint16_t get_duration_from_note_value(const NumberedNote *note, TimeSignature time_sig, uint8_t bpm);
void convert_numbered_to_note(const NumberedNote *numbered_melody, Note *melody, uint16_t length, TimeSignature time_sig, uint8_t bpm);
void Play_Numbered_Melody(const NumberedNote *numbered_melody, uint16_t length, TimeSignature time_sig, uint8_t bpm);
TimeSignatureInfo get_time_signature_info(TimeSignature time_sig);


#endif
