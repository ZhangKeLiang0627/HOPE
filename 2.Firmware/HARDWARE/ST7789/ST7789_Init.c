#include "ST7789_Init.h"
#include "spi.h"
#include "MyDMA.h"
#include "delay.h"

void ST7789_GPIO_Init(void)
{
	/*初始化结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*开启rcc时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

	/*初始化引脚口PA15-RES&PC14-DC&PC15-BKLIGHT*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*设置成高电平*/
	GPIO_SetBits(GPIOA, GPIO_Pin_15);
	GPIO_SetBits(GPIOC, GPIO_Pin_14 | GPIO_Pin_15);
}

/**
 * @brief LCD写入数据（8bit）
 * @param
 * @retval
 */
void LCD_WR_DATA8(u8 dat)
{
	LCD_Writ_Bus(dat);
}

/**
 * @brief LCD写入数据（16bit）
 * @param
 * @retval
 */
void LCD_WR_DATA(u16 dat)
{
	LCD_Writ_Bus(dat >> 8);
	LCD_Writ_Bus(dat);
}

/**
 * @brief LCD写入命令
 * @param
 * @retval
 */
void LCD_WR_REG(u8 dat)
{
	LCD_DC_Clr(); // 写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set(); // 写数据
}

/**
  * @brief 设置起始和结束地址
  * @param  x1,x2	设置列起始和结束地址
			y1,y2 	设置行起始和结束地址
  * @retval 无
  */
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
	if (USE_HORIZONTAL == 0)
	{
		LCD_WR_REG(0x2a); // 列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); // 行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); // 储存器写
	}
	else if (USE_HORIZONTAL == 1)
	{
		LCD_WR_REG(0x2a); // 列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); // 行地址设置
		LCD_WR_DATA(y1 + 80);
		LCD_WR_DATA(y2 + 80);
		LCD_WR_REG(0x2c); // 储存器写
	}
	else if (USE_HORIZONTAL == 2)
	{
		LCD_WR_REG(0x2a); // 列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); // 行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); // 储存器写
	}
	else
	{
		LCD_WR_REG(0x2a); // 列地址设置
		LCD_WR_DATA(x1 + 80);
		LCD_WR_DATA(x2 + 80);
		LCD_WR_REG(0x2b); // 行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); // 储存器写
	}
}

u8 SendBuff[1]; // 2*240

void ST7789_Init(void)
{
	/*初始化*/
	SPI1_Init();
	ST7789_GPIO_Init();

	DMA2_Init(240 * 240);

	LCD_RES_Clr(); // 复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);

	LCD_BLK_Set(); // 打开背光
	delay_ms(100);

	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); // Sleep out
	delay_ms(120);	  // Delay 120ms
	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x36);
	if (USE_HORIZONTAL == 0)
		LCD_WR_DATA8(0x00);
	else if (USE_HORIZONTAL == 1)
		LCD_WR_DATA8(0xC0);
	else if (USE_HORIZONTAL == 2)
		LCD_WR_DATA8(0x70);
	else
		LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x32); // Vcom=1.35V

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x15); // GVDD=4.8V  颜色深度

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20); // VDV, 0x20:0v

	LCD_WR_REG(0xC6);
	LCD_WR_DATA8(0x0F); // 0x0F:60Hz

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x0E);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x48);
	LCD_WR_DATA8(0x17);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x34);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x0E);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x48);
	LCD_WR_DATA8(0x17);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x34);
	LCD_WR_REG(0x21);

	LCD_WR_REG(0x29);
}
