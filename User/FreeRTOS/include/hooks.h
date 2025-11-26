#ifndef _HOOKS_H_
#define _HOOKS_H_
\

#include <stm32f4xx.h>
#include <FreeRTOS.h>
#include <task.h>
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );

void vApplicationMallocFailedHook( void );


#endif
