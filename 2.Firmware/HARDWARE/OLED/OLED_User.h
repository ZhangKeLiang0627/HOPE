#ifndef __OLED_USER_H
#define __OLED_USER_H

#include "stm32f4xx.h" // Device header

/*u8g2_SetDrawColor param*/
#define NORMAL 1
#define XOR 2

/* API */
void Oled_u8g2_ShowFloat(uint16_t x, uint16_t y, float dat, uint8_t num, uint8_t pointNum);
void Oled_u8g2_ShowNum(uint16_t x, uint16_t y, uint16_t dat);
void Oled_u8g2_ShowStr(uint16_t x, uint16_t y, char *str);
void Oled_u8g2_ShowUTF8(uint16_t x, uint16_t y, char *str);
void Oled_u8g2_DrawPoint(uint16_t x, uint16_t y);
void Oled_u8g2_DrawBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void Oled_u8g2_DrawFrame(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void Oled_u8g2_DrawRFrame(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t r);
void Oled_u8g2_DrawRBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t r);
void Oled_u8g2_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void Oled_u8g2_ClearBuffer(void);
void Oled_u8g2_SendBuffer(void);
void Oled_u8g2_SetDrawColor(uint8_t mode);
void Oled_u8g2_ShowBMP(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *pic);
void Oled_u8g2_ModifyColor(uint8_t mode);
uint32_t Oled_u8g2_Get_UTF8_ASCII_PixLen(char *s);
void Oled_u8g2_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
uint16_t Oled_u8g2_GetBufferTilHeight(void);
uint16_t Oled_u8g2_GetBufferTileWidth(void);
uint8_t *Oled_u8g2_GetBufferPtr(void);
void Oled_u8g2_SetBitmapMode(uint8_t mode);
void Oled_u8g2_SetFlipMode(uint8_t mode);

void Oled_u8g2_SetFont(const uint8_t *font);
void Oled_SetOledInverseColor(uint8_t flag);

void Oled_DrawIntensiveComputing(void);
void Oled_DrawIntensiveComputingLine(void);
void Oled_DrawSlowBitmapResize(int x, int y, const uint8_t *bitmap, int w1, int h1, int w2, int h2);
void Oled_DrawSlowBitmapResize2(int x, int y, const uint8_t *bitmap, int w1, int h1, int w2, int h2);
void Oled_ScreenshotPrint(void);

/* Event */
void Oled_EventUpdateOledLightLevel(void);
void Oled_EventOledInverseColor(void);

#endif
