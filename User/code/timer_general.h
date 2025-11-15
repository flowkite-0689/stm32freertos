#ifndef TIMER_GENERAL_H
#define TIMER_GENERAL_H
#include "stm32f4xx.h"
#include "sys.h"


void TIM13_PWM_Init(void);
void Set_PWM_Percentage(uint8_t percentage);
void Set_PWM_Frequency(uint32_t frequency);

#endif

