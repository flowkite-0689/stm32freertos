#ifndef _ALARM_MENU_H
#define _ALARM_MENU_H

#include "oled.h"
#include "oled_print.h"
#include "key.h"
#include "logo.h"
#include "alarm.h"

#define alarm_menu_options_NUM 2

void alarm_menu(void);

// 新增函数声明
void alarm_create(void);
void alarm_list(void);

#endif
