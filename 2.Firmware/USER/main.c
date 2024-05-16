// 系统
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "Random.h"

// 硬件驱动
#include "led.h"
#include "WS2812B.h"
#include "Encoder.h"
#include "BEEPER.h"
#include "oled.h"
#include "multi_button_user.h"

// 内存管理
#include "malloc.h"
#include "sdio_sdcard.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "fattester.h"

// UI
#include "u8g2.h"
#include "u8g2_Init.h"
#include "HugoUI_User.h"
#include "Hugo_UI.h"
#include "FileSystem.h"


// 控制按键、编码器
uint8_t KeyNum = 0;
uint8_t EncoderNum = 0;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置系统中断优先级分组2
	delay_init(84);									// 初始化延时函数
	USART6_Init(115200);							// 初始化串口6波特率为115200
	uart_init(115200);								// 初始化串口1波特率为115200

	TIM4_Random_Init(8400 - 1, 10 - 1); // 1ms定时器初始化
	user_keyBSP_init();					// 按键初始化
	Encoder_Init();						// 编码器初始化
	LED_Init();							// led初始化
	OLED_Init();						// i2c初始化c
	Beeper_Init();						// 蜂鸣器初始化
	Beeper_Perform(BEEPER_TRITONE);		// 蜂鸣器响声
	PWM_WS2812B_Init(WS2812B_ARR - 1);	// RGB初始化

	my_mem_init(SRAMIN); // 初始化内部内存池
	exfuns_init();		 // 为fatfs相关变量申请内存

	FileSystem_Init(); // 初始化sd卡和flash并挂载文件系统

	// HugoUI_InitLayout();

	HugoUIAnimationInit(); // HugoUI初始化

	EventLoadSettingConfig(); // 从flash读取保存的数据

	WS2812B_Write_24Bits(1, 0x050001); // RGB亮灯
	WS2812B_Show();

	while (1)
	{

		HugoUI_System();

	}
}
