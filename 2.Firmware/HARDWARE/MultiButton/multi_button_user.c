#include "multi_button_user.h"
#include "multi_button.h"

/*user_add_header_file*/
#include "stm32f4xx.h"
#include "usart.h"
#include "Beeper.h"
/*user_add*/

/*user_add_define*/
#define ENCODER_MODE 1
/*user_add_define*/

/*user_add_handle*/
struct Button key0;
struct Button key1;
struct Button key2;
/*user_add_handle*/

/*user_add_param*/
extern uint8_t KeyNum;
extern uint8_t EncoderNum;
uint8_t EncoderMode_flag = 1;
/*user_add_param*/

/*user_add_funtion*/
void user_keyBSP_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

#if ENCODER_MODE == 0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	button_init(&key0, read_key0_gpio, 0, 0);
	button_init(&key1, read_key1_gpio, 0, 1);

	button_attach(&key0, PRESS_DOWN, key0_press_down_Handler);
	button_attach(&key0, PRESS_UP, key0_press_up_Handler);
	button_attach(&key0, LONG_PRESS_START, key0_long_press_start_Handler);
	button_attach(&key0, SINGLE_CLICK, key0_single_click_Handler);

	button_attach(&key1, PRESS_DOWN, key1_press_down_Handler);
	button_attach(&key1, PRESS_UP, key1_press_up_Handler);
	button_attach(&key1, LONG_PRESS_START, key1_long_press_start_Handler);
	button_attach(&key1, SINGLE_CLICK, key1_single_click_Handler);

	button_start(&key0);
	button_start(&key1);

#else
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	button_init(&key2, read_key2_gpio, 0, 0);
	button_attach(&key2, LONG_PRESS_START, key2_long_press_start_Handler);
	button_attach(&key2, SINGLE_CLICK, key2_single_click_Handler);
	button_start(&key2);
#endif
}

uint8_t read_key0_gpio(void)
{
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
}

uint8_t read_key1_gpio(void)
{
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);
}

uint8_t read_key2_gpio(void)
{
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2);
}

void key0_press_down_Handler(void *btn)
{
	printf("---> key0 press down! <---\r\n");

	Beeper_Perform(BEEPER_KEYPRESS);
}

void key0_press_up_Handler(void *btn)
{
	// EncoderNum = 1;
	printf("***> key0 press up! <***\r\n");
}

void key0_single_click_Handler(void *btn)
{

	EncoderNum = 1;
}

void key0_long_press_start_Handler(void *btn)
{
	KeyNum = 1;

	Beeper_Perform(BEEPER_TRITONE);
}

void key1_press_down_Handler(void *btn)
{
	printf("---> key1 press down! <---\r\n");

	Beeper_Perform(BEEPER_KEYPRESS);
}

void key1_press_up_Handler(void *btn)
{
	// EncoderNum = 2;
	printf("***> key1 press up! <***\r\n");
}

void key1_single_click_Handler(void *btn)
{
	EncoderNum = 2;
}

void key1_long_press_start_Handler(void *btn)
{
	KeyNum = 2;
	Beeper_Perform(BEEPER_WARNING);
}

void key2_single_click_Handler(void *btn)
{
	KeyNum = 1;
	Beeper_Perform(BEEPER_TRITONE);
}

void key2_long_press_start_Handler(void *btn)
{
	KeyNum = 2;
	Beeper_Perform(BEEPER_WARNING);
}
/*user_add_funtion*/
