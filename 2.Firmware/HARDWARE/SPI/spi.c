#include "spi.h"
#include "delay.h"
// 以下是SPI模块的初始化代码，配置成主机模式
// SPI口初始化
// 这里针是对SPI2的初始化
void SPI2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // 使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);  // 使能SPI2时钟

	// GPIOB初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // PB13~15复用功能输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;						   // 复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;						   // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					   // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;						   // 上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);								   // 初始化

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2); // PB13复用为 SPI2
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2); // PB14复用为 SPI2
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2); // PB15复用为 SPI2

	// 这里只针对SPI口初始化
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);  // 复位SPI2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE); // 停止复位SPI2

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	 // 设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						 // 设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					 // 设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;							 // 串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;						 // 串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							 // NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; // 定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;					 // 指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;							 // CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);									 // 根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	SPI_Cmd(SPI2, ENABLE); // 使能SPI外设

	SPI2_ReadWriteByte(0xff); // 启动传输
}
// SPI2速度设置函数
// SpeedSet:0~7
// SPI速度=fAPB2/2^(SpeedSet+1)
// fAPB2时钟一般为84Mhz
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1 &= 0XFFC7;
	SPI2->CR1 |= SPI_BaudRatePrescaler; // 设置SPI1速度
	SPI_Cmd(SPI2, ENABLE);
}
// SPI2 读写一个字节
// TxData:要写入的字节
// 返回值:读取到的字节
u8 SPI2_ReadWriteByte(u8 TxData)
{

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	{
	} // 等待发送区空

	SPI_I2S_SendData(SPI2, TxData); // 通过外设SPIx发送一个byte数据

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
	{
	} // 等待接收完一个byte

	return SPI_I2S_ReceiveData(SPI2); // 返回通过SPIx最近接收的数据
}

/**
 * @brief SPI2写入一个字节
 *
 * @param TxData 需要写入的字节
 * @return uint8_t 返回是否成功
 */
void SPI2_WriteByte(uint8_t TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
		; // 等待发送区空

	SPI_I2S_SendData(SPI2, TxData); // 通过外设SPIx发送一个byte数据

}

/**
 * @brief SPI2读取一个字节
 *
 * @param none
 * @return uint8_t 返回读取到的字节
 */
uint8_t SPI2_ReadByte(void)
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
		; // 等待接收完一个byte

	return SPI_I2S_ReceiveData(SPI2); // 返回通过SPIx最近接收的数据
}

void SPI1_Init(void)
{
	/*初始化结构体*/
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/*开启rcc时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/*初始化PA5-SPI1_SCK&PA6-SPI1_MISO&PA7-SPI1_MISO&PA15-sw_CS*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); // PA15-SPI_CS

	/*复用PA5为SPI1_SCK&PA6为SPI1_MISO&PA7为SPI1_MOSI*/
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	/*初始化SPI1*/
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // SPI_Direction_1Line_Tx
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI1, &SPI_InitStructure);

	/*打开SPI1_TX和DMA的传输通道*/
	//SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);

	/*开启SPI1*/
	SPI_Cmd(SPI1, ENABLE);
}

void LCD_Writ_Bus(u8 dat)
{
	LCD_CS_Clr();

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_I2S_SendData(SPI1, dat);

	delay_us(1); // 这个delay很重要不能删去
	LCD_CS_Set();
}

/**
 * @brief SPI1写入一个字节
 *
 * @param TxData 需要写入的字节
 * @return uint8_t 返回是否成功
 */
void SPI1_WriteByte(uint8_t TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		; // 等待发送区空

	SPI_I2S_SendData(SPI1, TxData); // 通过外设SPIx发送一个byte数据
}

/**
 * @brief SPI1读取一个字节
 *
 * @param none
 * @return uint8_t 返回读取到的字节
 */
uint8_t SPI1_ReadByte(void)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		; // 等待接收完一个byte

	return SPI_I2S_ReceiveData(SPI1); // 返回通过SPIx最近接收的数据
}

// SPI1 读写一个字节
// TxData:要写入的字节
// 返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
	{
	} // 等待发送区空

	SPI_I2S_SendData(SPI1, TxData); // 通过外设SPIx发送一个byte数据

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
	{
	} // 等待接收完一个byte

	return SPI_I2S_ReceiveData(SPI1); // 返回通过SPIx最近接收的数据
}
