#ifndef __TEXT_H__
#define __TEXT_H__
#include <stm32f4xx.h>
#include "fontupd.h"
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////

void Get_HzMat(unsigned char *code, unsigned char *mat, u8 size);              // 得到汉字的点阵码
void Show_Font(u16 x, u16 y, u8 *font, u8 size, u8 mode);                      // 在指定位置显示一个汉字
void Show_Str(u16 x, u16 y, u16 width, u16 height, u8 *str, u8 size, u8 mode); // 在指定位置显示一个字符串
void Show_Str_Mid(u16 x, u16 y, u8 *str, u8 size, u8 len);

void Show_Str_forOled(u16 x, u16 y, u16 width, u16 height, u8 *str, u8 size, u8 mode);
#endif
