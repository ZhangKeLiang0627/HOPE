#include "oled.h"

void Oled_I2C_WaitEvent(I2C_TypeDef *I2Cx, uint32_t I2C_EVENT)
{
	uint16_t Timeout;
	Timeout = 10000;
	while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS)
	{
		Timeout--;
		if (Timeout == 0)
		{
			break;
		}
	}
}

void I2C_WriteByte(uint8_t addr, uint8_t data)
{
	// while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2C1, ENABLE);
	Oled_I2C_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);

	I2C_Send7bitAddress(I2C1, OLED_ADDRESS, I2C_Direction_Transmitter);
	Oled_I2C_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);

	I2C_SendData(I2C1, addr);
	Oled_I2C_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);

	I2C_SendData(I2C1, data);
	Oled_I2C_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);

	I2C_GenerateSTOP(I2C1, ENABLE);
}

void WriteCmd(unsigned char I2C_Command) // 写命令
{
	I2C_WriteByte(0x00, I2C_Command); //
}

void WriteDat(unsigned char I2C_Data) // 写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}

// 反显函数
void OLED_ColorTurn(u8 i)
{
	if (i == 0)
	{
		WriteCmd(0xA6);
	}
	if (i == 1)
	{
		WriteCmd(0xA7);
	}
}

// 屏幕旋转180度
void OLED_DisplayTurn(u8 i)
{
	if (i == 0)
	{
		WriteCmd(0xC8);
		WriteCmd(0xA1);
	}
	if (i == 1)
	{
		WriteCmd(0xC0);
		WriteCmd(0xA0);
	}
}

// 开启OLED显示
void OLED_DisPlay_On(void)
{
	WriteCmd(0x8D);
	WriteCmd(0x14);
	WriteCmd(0xAF);
}

// 关闭OLED显示
void OLED_DisPlay_Off(void)
{
	WriteCmd(0x8D);
	WriteCmd(0x10);
	WriteCmd(0xAE);
}

// OLED的初始化
void OLED_Init(void)
{
	/*初始化结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	/*打开rcc时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	/*初始化PB6&PB7*/
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	   // 复用
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	   // 开漏
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*PB6-I2C1_SCK&PB7-I2C1_SDA*/
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

	/*初始化HWI2C*/
	I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED; // 400Khz//bigger is also ok
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x77; // 主机地址

	I2C_Init(I2C1, &I2C_InitStructure);

	/*开启I2C1*/
	I2C_Cmd(I2C1, ENABLE);
}
