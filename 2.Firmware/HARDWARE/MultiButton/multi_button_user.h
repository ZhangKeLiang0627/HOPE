#ifndef __MULTI_BUTTON_USER_H
#define __MULTI_BUTTON_USER_H
#include "stdint.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif
/*user_add*/
void user_keyBSP_init(void);

uint8_t read_key0_gpio(void);

uint8_t read_key1_gpio(void);

uint8_t read_key2_gpio(void);

void key0_press_down_Handler(void *btn);

void key0_press_up_Handler(void* btn);

void key0_single_click_Handler(void *btn);

void key0_long_press_start_Handler(void* btn);
	
void key1_press_down_Handler(void* btn);

void key1_press_up_Handler(void* btn);

void key1_single_click_Handler(void *btn);

void key1_long_press_start_Handler(void *btn);

void key2_single_click_Handler(void *btn);

void key2_long_press_start_Handler(void *btn);
/*user_add*/

#ifdef __cplusplus
}
#endif
		 
#endif
