#ifndef __U8G2_INIT_H__
#define __U8G2_INIT_H__
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void u8g2Init(u8g2_t *u8g2);

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void HW_I2cWrite(uint8_t *buf,uint8_t len);

#endif



