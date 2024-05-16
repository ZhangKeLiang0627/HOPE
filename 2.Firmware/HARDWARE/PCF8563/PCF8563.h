#ifndef __PCF8563_H
#define __PCF8563_H
#include "stm32f4xx.h"
//---寄存器定义
#define PCF8563_REG_CONTROL1 0x00
#define PCF8563_REG_CONTROL2 0x01
#define PCF8563_REG_SECONDS 0x02
#define PCF8563_REG_MINUTES 0x03
#define PCF8563_REG_HOURS 0x04
#define PCF8563_REG_DAYS 0x05
#define PCF8563_REG_WEEK 0x06
#define PCF8563_REG_MONTHS 0x07
#define PCF8563_REG_YEARS 0x08
#define PCF8563_REG_MINUTES_ALARM 0x09
#define PCF8563_REG_HOURS_ALARM 0x0A
#define PCF8563_REG_DAYS_ALARM 0x0B
#define PCF8563_REG_WEEK_ALARM 0x0C
#define PCF8563_REG_CLKOUT 0x0D
#define PCF8563_REG_TIMER_CONTROL 0x0E
#define PCF8563_REG_TIMER 0x0F

//---I2C设备地址
#define PCF8563_IIC_ADDR 0x51 // write 0x51 << 1[0xa2] // read 0x51 << 1 + 1[0xa3]
// #define PCF8563_WRITE_ADDR 0xA2
// #define PCF8563_READ_ADDR 0xA3

//---CLKOUT的输出
#define PCF8563_CLKOUT_32768HZ 0
#define PCF8563_CLKOUT_1024HZ 1
#define PCF8563_CLKOUT_32HZ 2
#define PCF8563_CLKOUT_1HZ 3

/*PCF8563_RTC的结构体*/
typedef struct
{
    uint8_t sec;   // 秒
    uint8_t min;   // 分
    uint8_t hour;  // 时
    uint8_t date;  // 日期
    uint8_t month; // 月份
    uint16_t year; // 年份
    uint8_t day;   // 星期

} PCF8563_Tag;

extern PCF8563_Tag RTC_Time;

uint8_t PCF8563_Init(void);
uint8_t PCF8563_GetTime(void);
void PCF8563_SetTime(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec);

#endif
