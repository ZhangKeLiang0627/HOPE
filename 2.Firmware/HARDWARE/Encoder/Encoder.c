#include "Encoder.h"
#include "BEEPER.h"
void Encoder_Init(void)
{
    /*初始化结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    /*开启rcc时钟*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /*初始化PB4&PB5*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*将PB4复用为TIM3_CH1&将PB5复用为TIM3_CH2*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);

    /*TIM3初始化*/
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 65535;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    /*输入捕获初始化*/
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 15; // 输入捕获滤波器（0~15）
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    /*配置编码器函数*/
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI1, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    /*初始化计数值*/
    TIM_SetCounter(TIM3, 32768);

    /*开启定时器TIM3*/
    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief 读取CE11的值
 * @param  无
 * @retval 返回差值
 */
static int16_t Encoder_ReadValue(void)
{
    int16_t value = 0;

    value = (int16_t)(TIM_GetCounter(TIM3) - 32768);
    TIM_SetCounter(TIM3, 32768);

    return value;
}

/**
 * @brief 应用层函数（在主函数中调用）
 * @param  无
 * @retval 正转返回1/反转返回2/无状态返回0
 */
uint8_t Encoder_GetNum(void)
{
    int8_t Encoder_Num = 0;
    static uint8_t Encoder_ReturnNum1, Encoder_ReturnNum2;

    /* 读取输入值 */
    Encoder_Num = Encoder_ReadValue();
    /* 没有输入就返回0 */
    if (Encoder_Num == 0)
        return 0;
    // else
    //     printf("Encoder:%d\r\n", Encoder_Num);

    if (Encoder_Num > 0)
    {
        Encoder_ReturnNum1 += Encoder_Num;
    }
    else
    {
        Encoder_ReturnNum2 += (-Encoder_Num);
    }

    if (Encoder_ReturnNum1 >= 2)
    {
        Encoder_ReturnNum1 = 0;
        Encoder_ReturnNum2 = 0;
        Beeper_Perform(BEEPER_KEYPRESS);
        return 1;
    }
    else if (Encoder_ReturnNum2 >= 2 || (Encoder_ReturnNum1 == 1 && Encoder_ReturnNum2 == 1))
    {
        Encoder_ReturnNum1 = 0;
        Encoder_ReturnNum2 = 0;
        Beeper_Perform(BEEPER_KEYPRESS);
        return 2;
    }
    else
        return 0;
}

extern uint8_t EncoderNum;
void Encoder_Foreward_CallBack(void);
void Encoder_Reverse_CallBack(void);

/**
 * @brief 应用层函数（在主函数中调用）
 * @param  无
 * @retval 正转/反转会触发CallBack
 */
void Encoder_Handler(void)
{
    int8_t Encoder_Num = 0;
    static uint8_t Encoder_ReturnNum1, Encoder_ReturnNum2;

    /* 读取输入值 */
    Encoder_Num = Encoder_ReadValue();
    /* 没有输入就返回0 */
    if (Encoder_Num == 0)
        return ;
    // else
    //     printf("Encoder:%d\r\n", Encoder_Num);

    if (Encoder_Num > 0)
    {
        Encoder_ReturnNum1 += Encoder_Num;
    }
    else
    {
        Encoder_ReturnNum2 += (-Encoder_Num);
    }

    if (Encoder_ReturnNum1 >= 2)
    {
        Encoder_ReturnNum1 = 0;
        Encoder_ReturnNum2 = 0;
        Encoder_Foreward_CallBack();
    }
    else if (Encoder_ReturnNum2 >= 2 || (Encoder_ReturnNum1 == 1 && Encoder_ReturnNum2 == 1))
    {
        Encoder_ReturnNum1 = 0;
        Encoder_ReturnNum2 = 0;
        Encoder_Reverse_CallBack();
    }
}


void Encoder_Foreward_CallBack(void)
{
    EncoderNum = 1;
    Beeper_Perform(BEEPER_KEYPRESS);
}
void Encoder_Reverse_CallBack(void)
{
    EncoderNum = 2;
    Beeper_Perform(BEEPER_KEYPRESS);
}
