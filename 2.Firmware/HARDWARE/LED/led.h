#ifndef __LED_H
#define __LED_H
#include "stm32f4xx.h" 

// LED端口定义
#define LED0 PCout(13) // LED0

void LED_Init(void); // LED初始化
#endif
