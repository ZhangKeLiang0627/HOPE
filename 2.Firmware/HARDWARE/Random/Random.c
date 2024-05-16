#include "Random.h"

#include "Encoder.h"
#include "multi_button.h"
#include "BEEPER.h"
#include "Hugo_UI.h"

uint32_t Time_ms = 0; // 毫秒计数

/**
 * @brief 初始化TIM4(已经配置好优先级了)
 * @param  ARR(重装载值）//PSC(预分频值)
 * @retval 无
 */
void TIM4_Random_Init(uint16_t ARR, uint16_t PSC)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruture;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseInitStruture.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruture.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruture.TIM_Period = ARR;	  // ARR = 8400 - 1	//1ms
	TIM_TimeBaseInitStruture.TIM_Prescaler = PSC; // RSC = 10 - 1
	TIM_TimeBaseInitStruture.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruture);

	TIM_ClearFlag(TIM4, TIM_FLAG_Update);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief 产生一个0到1000的随机数
 * @param  无
 * @retval 返回这个得到的随机数
 */
uint32_t RandomCreate(void)
{
	uint32_t Num = 0;

	srand(Time_ms);

	Num = rand() % 1000; // 产生一个0到100的随机数//改成一亿先

	return Num;
}

uint8_t Key_count;
uint16_t NET_count;
uint8_t NET_flag;
uint8_t Encoder_count;
uint16_t Beeper_count;

/**
 * @brief TIM4中断（ms计数器）
 * @param  无
 * @retval 无
 */
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		Time_ms++;

		/* ---------- HugoUI_Ticks ---------- */
		HugoUI_Ticks();
		
		/*---------- multi_button ----------*/
		if (++Key_count >= 5)
		{
			button_ticks();
			Key_count = 0;
		}
		/* ---------- Encoder ---------- */
		if (++Encoder_count >= 10)
		{
			Encoder_Handler();
			Encoder_count = 0;
		}
		/* ---------- Beeper ---------- */
		if (++Beeper_count >= 10)
		{
			Beeper_Proc();
			Beeper_count = 0;
		}
		/* ---------- NET ---------- */
		if (++NET_count >= 5000)
		{
			NET_flag = 1;
			NET_count = 0;
		}
	}
}
