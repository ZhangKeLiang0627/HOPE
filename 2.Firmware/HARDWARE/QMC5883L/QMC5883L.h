#ifndef __QMC5883L_H
#define __QMC5883L_H

#include "Ellipsoid fitting.h"
// QMC5883L寄存器列表
#define QMC5883L_DATA_X_LSB 0x00       // X LSB			只读
#define QMC5883L_DATA_X_MSB 0x01       // X MSB			只读
#define QMC5883L_DATA_Y_LSB 0x02       // Y LSB			只读
#define QMC5883L_DATA_Y_MSB 0x03       // Y MSB			只读
#define QMC5883L_DATA_Z_LSB 0x04       // Z LSB			只读
#define QMC5883L_DATA_Z_MSB 0x05       // Z MSB			只读
#define QMC5883L_STATUS_REG 0x06       // 状态寄存器	只读
#define QMC5883L_TEMP_LSB 0x07         // 温度 LSB		只读-温度只是相对值，没有实际用途
#define QMC5883L_TEMP_MSB 0x08         // 温度 MSB		只读-温度只是相对值，没有实际用途
#define QMC5883L_CONTROL1_REG 0x09     // 控制寄存器1	读写
#define QMC5883L_CONTROL2_REG 0x0A     // 控制寄存器2	读写-BIT7为复位
#define QMC5883L_SET_RESET_PERIOD 0x0B // 时间寄存器，设置为0x01
#define QMC5883L_ID_REG 0x0D           // 芯片ID		只读

#define QMC5883L_ID 0xFF // QMC5883L 芯片ID

#define QMC5883L_IIC_ADDR 0x0d // 0x18 // 0x1A	  //定义器件在IIC总线中的从地址

unsigned char QMC5883L_Read_Byte(unsigned char addr);
unsigned char QMC5883LCheckConnection(void);
void QMC5883LRead(int *buf);
void QMC5883LSetMode(unsigned char mode);
unsigned char QMC5883LInit(void);
void QMC5883LSetCaliValue(tsCaliData *c);
void QMC5883LClearCaliValue(void);
float QMC5883LCalYaw(int *mag_data, float ax, float ay);

#endif
