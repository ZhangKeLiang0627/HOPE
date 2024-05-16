#ifndef __OLED_H
#define __OLED_H

#include "stm32f4xx.h"
#include "delay.h"

#define I2C_SPEED 1000000 // 1Mhz // 400kHz
#define OWN_ADDRESS 0X77
#define OLED_ADDRESS 0X78
#define OLED_I2C I2C1

void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);

void Oled_I2C_WaitEvent(I2C_TypeDef *I2Cx, uint32_t I2C_EVENT);
void I2C_WriteByte(uint8_t addr, uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);

void OLED_Init(void);
#endif
