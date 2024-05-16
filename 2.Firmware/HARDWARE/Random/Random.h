#ifndef __RANDOM_H__
#define __RANDOM_H__
#include "stm32f4xx.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void TIM4_Random_Init(uint16_t ARR, uint16_t PSC);

uint32_t RandomCreate(void);

#endif
