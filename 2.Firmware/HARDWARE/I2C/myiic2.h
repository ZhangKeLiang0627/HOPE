#ifndef __MYIIC2_H
#define __MYIIC2_H
#include "stm32f4xx.h"

void MyIIC_Init(void);
void MyIIC_Start(void);
void MyIIC_Stop(void);
void MyIIC_SendByte(uint8_t Byte);
uint8_t MyIIC_ReceiveByte(uint8_t ack);
void MyIIC_SendAck(uint8_t AckBit);
uint8_t MyIIC_ReceiveAck(void);
uint8_t MyIIC_Write_SingleByte(uint8_t SlaveAddress, uint8_t REG_Address, uint8_t REG_data);
uint8_t MyIIC_Read_SingleByte(uint8_t SlaveAddress, uint8_t REG_Address);
uint8_t MyIIC_Write_MultiBytes(uint8_t DeviceAddr, uint8_t REG_Address, uint8_t BytesNum, uint8_t *buf);
uint8_t MyIIC_Read_MultiBytes(uint8_t DeviceAddr, uint8_t REG_Address, uint8_t BytesNum, uint8_t *buf);
#endif
