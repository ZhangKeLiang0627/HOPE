#ifndef __SPI_H
#define __SPI_H
#include "stm32f4xx.h"

#define LCD_CS_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define LCD_CS_Set() GPIO_SetBits(GPIOA, GPIO_Pin_15)

void SPI2_Init(void);             // 初始化SPI2
void SPI2_SetSpeed(u8 SpeedSet);  // SPI2设置速率
u8 SPI2_ReadWriteByte(u8 TxData); // SPI2读写一个字节
void SPI2_WriteByte(uint8_t TxData);
uint8_t SPI2_ReadByte(void);

void SPI1_Init(void); // 初始化SPI1
void LCD_Writ_Bus(u8 dat);
void SPI1_WriteByte(uint8_t TxData);
uint8_t SPI1_ReadByte(void);
u8 SPI1_ReadWriteByte(u8 TxData);

#endif
