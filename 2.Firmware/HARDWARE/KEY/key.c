#include "key.h"
#include "delay.h"

// 按键初始化函数
void KEY_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // 使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		   // KEY0对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	   // 普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   // 上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);			   // 初始化GPIOA0
}
// 按键处理函数
// 返回按键值
// mode:0,不支持连续按;1,支持连续按;
// 0，没有任何按键按下
// 1，KEY0按下
// 2，KEY1按下
// 3，KEY2按下
// 4，WKUP按下 WK_UP
// 注意此函数有响应优先级,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{
	static u8 key_up = 1; // 按键按松开标志
	if (mode)
		key_up = 1; // 支持连按
	if (key_up && (KEY0 == 0))
	{
		delay_ms(10); // 去抖动
		key_up = 0;
		if (KEY0 == 0)
			return 1;
	}
	else if (KEY0 == 1)
		key_up = 1;
	return 0; // 无按键按下
}
