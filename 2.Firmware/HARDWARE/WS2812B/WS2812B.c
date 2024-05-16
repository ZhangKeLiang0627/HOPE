#include "sys.h"
#include "WS2812B.h"
#include "delay.h"

/*单个灯珠的需要传输的数据对应的比较值数组*/
uint32_t Single_WS2812B_Buffer[DATA_SIZE * WS2812B_NUM + 50] = {0}; // 24bit*灯珠数量 + Reset_Data(1.25us*50>50us)

/**
 * @brief TIM5_PWM_CH2&DMA1&PA1初始化
 * @param  arr重装载值（105 - 1）
 * @retval 无
 */
void PWM_WS2812B_Init(uint16_t arr)
{
	/*初始化结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	/*使能rcc时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	/*初始化gpio引脚口*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*复用PA2为TIM5_CH3*/
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM5);

	TIM_DeInit(TIM5);
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 时钟分割
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period = arr;						// arr
	TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;				// psc：1分频（不分频）
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			// 高级定时器专属

	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);
	TIM_CtrlPWMOutputs(TIM5, ENABLE);

	/*初始化TIM5_CH3 PWM模式*/
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;			  // 选择定时器模式:TIM脉冲宽度调制模式
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	  // 输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;

	TIM_OC3Init(TIM5, &TIM_OCInitStructure);

	TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable); // 使能TIM5在CCR3上的预装载寄存器

	TIM_Cmd(TIM5, DISABLE); // 失能TIM5，防止第一个脉冲异常

	TIM_DMACmd(TIM5, TIM_DMA_CC3, ENABLE); // 使能TIM5_CH3的DMA功能(CC3对应通道3)

	DMA_DeInit(DMA1_Stream0);
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_BufferSize = DATA_SIZE;
	DMA_InitStructure.DMA_Channel = DMA_Channel_6;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Single_WS2812B_Buffer;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (TIM5->CCR3);
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA1_Stream0, &DMA_InitStructure);

	DMA_Cmd(DMA1_Stream0, DISABLE); // 失能DMA1的6通道，因为一旦使能就开始传输
}

void WS2812B_Write_24Bits(uint16_t num, uint32_t GRB_Data)
{
	uint8_t i, j;
	for (j = 0; j < num; j++)
	{
		for (i = 0; i < DATA_SIZE; i++)
		{
			/*因为数据发送的顺序是GRB，高位先发，所以从高位开始判断，判断后比较值先放入缓存数组*/
			Single_WS2812B_Buffer[i + j * DATA_SIZE] = ((GRB_Data << i) & 0x800000) ? T1H : T0H;
		}
	}
}

void WS2812B_Write_24Bits_independence(uint16_t num, uint32_t *GRB_Data)
{
	uint8_t i, j;
	for (j = 0; j < num; j++)
	{
		for (i = 0; i < DATA_SIZE; i++)
		{
			/*因为数据发送的顺序是GRB，高位先发，所以从高位开始判断，判断后比较值先放入缓存数组*/
			Single_WS2812B_Buffer[i + j * DATA_SIZE] = ((GRB_Data[j] << i) & 0x800000) ? T1H : T0H;
		}
	}
}

void WS2812B_Show(void)
{
	/* 移植时此处对应的通道和中断标志都需要更改 */

	DMA_SetCurrDataCounter(DMA1_Stream0, DATA_SIZE * WS2812B_NUM + 50);

	/*开启DMA和TIM5开始传输*/
	DMA_Cmd(DMA1_Stream0, ENABLE);
	TIM_Cmd(TIM5, ENABLE);

	while (DMA_GetFlagStatus(DMA1_Stream0, DMA_FLAG_TCIF0) == RESET)
		;

	DMA_ClearFlag(DMA1_Stream0, DMA_FLAG_TCIF0);
	DMA_Cmd(DMA1_Stream0, DISABLE);
	TIM_Cmd(TIM5, DISABLE);
}

// N个灯珠发红光
void PWM_WS2812B_Red(uint16_t num)
{
	WS2812B_Write_24Bits(num, 0x00ff00);
	WS2812B_Show();
}
// N个灯珠发绿光
void PWM_WS2812B_Green(uint16_t num)
{
	WS2812B_Write_24Bits(num, 0xff0000);
	WS2812B_Show();
}
// N个灯珠发蓝光
void PWM_WS2812B_Blue(uint16_t num)
{
	WS2812B_Write_24Bits(num, 0x0000ff);
	WS2812B_Show();
}

/*呼吸灯*/
void set_ws2812_breathing(uint8_t index)
{
	int i = 0;
	switch (index)
	{
	case 0: /* red */
		for (i = 0; i < 254; i += 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | i << 8 | 0x00));
			WS2812B_Show();
			delay_ms(10);
		}
		for (i = 254; i > 0; i -= 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | i << 8 | 0x00));
			WS2812B_Show();
			delay_ms(10);
		}
		break;
	case 1: /* green */
		for (i = 0; i < 254; i += 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(i << 16 | 0x00 << 8 | 0x00));
			WS2812B_Show();
			delay_ms(10);
		}
		for (i = 254; i > 0; i -= 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(i << 16 | 0x00 << 8 | 0x00));
			WS2812B_Show();
			delay_ms(10);
		}
		break;
	case 2: /* blue */
		for (i = 0; i < 254; i += 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | 0x00 << 8 | i));
			WS2812B_Show();
			delay_ms(10);
		}
		for (i = 254; i > 0; i -= 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | 0x00 << 8 | i));
			WS2812B_Show();
			delay_ms(10);
		}
		break;
	}
}
