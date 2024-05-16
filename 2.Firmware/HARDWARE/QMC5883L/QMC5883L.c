#include "QMC5883L.h"
#include "myiic2.h"
#include "Delay.h"
#include "math.h"
#include "usart.h"

/******************************设置参数*************************************/
unsigned char QMC5883L_OSR = 0x00;	// Over Sample Ratio // 过采样
									// 0x00 = 512; 0x01 = 256; 0x02 = 128; 0x03 = 64
unsigned char QMC5883L_ODR = 0x01;	// Typical Data Output Rate (Hz) // 更新速率
									// 0x00	10Hz
									// 0x01	50Hz
									// 0x02	100Hz
									// 0x03	200Hz
unsigned char QMC5883L_RNG = 0x00;	// Full Scale // 测量范围
									// 0x00	2G
									// 0x01	8G
unsigned char QMC5883L_MODE = 0x01; // Operating Mode // 工作模式
									// 0x00 = Standby
									// 0x01 = Continuous(Default).
/*****************************校准参数************************************/
int mag_x_offset = 0, mag_y_offset = 0, mag_z_offset = 0; // 零位校准参数
float mag_y_scale = 1, mag_z_scale = 1;					  // 幅度校准参数
/*************************************************************************/

/**
 * @brief  使用IIC总线往QMC5883L的寄存器中写一字节数据
 * @param  addr: 寄存器的地址
 * @param  dat: 	待写入的数据
 * @retval None
 */
void QMC5883L_Write_Byte(unsigned char addr, unsigned char dat)
{
	MyIIC_Write_SingleByte(QMC5883L_IIC_ADDR, addr, dat);
}
/**
 * @brief  使用IIC总线从QMC5883L的寄存器中读一字节数据
 * @param  addr: 寄存器的地址
 * @retval 读出的一字节数据
 */
unsigned char QMC5883L_Read_Byte(unsigned char addr)
{
	return MyIIC_Read_SingleByte(QMC5883L_IIC_ADDR, addr);
}
/**
 * @brief  检查与QMC5883L的连接是否正常
 * @param  无
 * @retval 1（成功），0（失败）
 */
unsigned char QMC5883LCheckConnection(void)
{
	if (QMC5883L_Read_Byte(0x0D) != 0xff)
		return 0;
	else
		return 1;
}

/**
 * @brief 	从QMC5883L中读取读传感器原始数据
 * @param  buf：原始数据数组的指针，int*型
 * @retval 无
 */
void QMC5883LRead(int *buf)
{
	unsigned char temp[6];

	MyIIC_Read_MultiBytes(QMC5883L_IIC_ADDR, 0x00, 6, temp);

	buf[0] = (temp[1] << 8) | temp[0];
	buf[1] = (temp[3] << 8) | temp[2];
	buf[2] = (temp[5] << 8) | temp[4];
	buf[0] = -buf[0];
	buf[0] -= mag_x_offset;
	buf[1] -= mag_y_offset;
	buf[2] -= mag_z_offset;
	buf[1] *= mag_y_scale;
	buf[2] *= mag_z_scale;
}
/**
 * @brief 	设置QMC5883L原始数据的校准参数
 * @param  tsCaliData *c: 校准参数的结构体指针
 * @retval 无
 */
void QMC5883LSetCaliValue(tsCaliData *c)
{
	mag_x_offset = c->X0;
	mag_y_offset = c->Y0;
	mag_z_offset = c->Z0;
	mag_y_scale = c->A / c->B;
	mag_z_scale = c->A / c->C;
}
/**
 * @brief 	清除QMC5883L的校准参数
 * @param  无
 * @retval 无
 */
void QMC5883LClearCaliValue(void)
{
	mag_x_offset = 0;
	mag_y_offset = 0;
	mag_z_offset = 0;
	mag_y_scale = 1;
	mag_z_scale = 1;
}
/**
 * @brief 	设置QMC5883L的工作模式
 * @param  mode：0	休眠模式
 *								1	连续测量模式
 * @retval 无
 */
void QMC5883LSetMode(unsigned char mode)
{
	QMC5883L_MODE = QMC5883L_Read_Byte(0x09);
	QMC5883L_MODE &= (~0x03);
	QMC5883L_Write_Byte(0x09, QMC5883L_MODE | mode);
}

/**
 * @brief 	QMC5883L初始化函数
 * @param  无
 * @retval 1：失败
 *		   0：成功
 */

uint8_t tempreg;

unsigned char QMC5883LInit(void)
{
	tempreg = (QMC5883L_OSR & 0x03 << 6) | (QMC5883L_RNG & 0x03 << 4) | (QMC5883L_ODR & 0x03 << 2) | (QMC5883L_MODE & 0x03);

	QMC5883L_Write_Byte(QMC5883L_CONTROL2_REG, 0X80); // 复位
	// delay_ms(100);

	QMC5883L_Write_Byte(QMC5883L_SET_RESET_PERIOD, 0X01); // 复位后必须写入0x01,否则温度为0，磁场数据也不对
	QMC5883L_Write_Byte(QMC5883L_CONTROL2_REG, 0x40);	  // 读数据时指针自动移到下一个寄存器地址
	QMC5883L_Write_Byte(QMC5883L_CONTROL1_REG, tempreg);  // 设置工作模式

	if (QMC5883L_Read_Byte(QMC5883L_ID_REG) != QMC5883L_ID)
	{
		printf("QMC5883LInitFlase, can't read ID OR read ID = 0x%x\r\n", QMC5883L_Read_Byte(QMC5883L_ID_REG));
		return 1;
	}
	// 读取配置，看看是否与写入的一致，由于芯片id是0xFF,并不能用于判断芯片是否初始化正常
	if (QMC5883L_Read_Byte(QMC5883L_CONTROL1_REG) != tempreg)
	{
		printf("QMC5883LInitFlase, can't read reg\r\n");
		return 1;
	}

	return 0; // 一切正常则返回0
}
/**
 * @brief 	根据x轴和y轴的倾角补偿磁偏角
 * @param  mag_data：三轴磁力数据
 *					ax，ay：x轴和y轴的倾角
 * @retval Yaw：补偿后的磁偏角弧度
 */
float QMC5883LCalYaw(int *mag_data, float ax, float ay)
{
	float Yaw, RadX, RadY, Xh, Yh;
	RadX = -ax / 57.2957795f;
	RadY = -ay / 57.2957795f;
	Xh = mag_data[0] * cosf(RadX) + mag_data[1] * sinf(RadY) * sinf(RadX) - mag_data[2] * cosf(RadY) * sinf(RadX);
	Yh = mag_data[1] * cosf(RadY) + mag_data[2] * sinf(RadY);
	Yaw = (float)atan2(Yh, Xh) * 180.0f / 3.1415926f + 180.0f;
	return Yaw;
}
