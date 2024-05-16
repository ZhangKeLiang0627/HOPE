#ifndef __ENCODER_H__
#define __ENCODER_H__
#include "stm32f4xx.h"
#include <stdio.h>

void Encoder_Init(void);

uint8_t Encoder_GetNum(void);
void Encoder_Handler(void);

#endif
