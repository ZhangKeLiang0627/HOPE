#include "BEEPER.h"


/*创建Beeper的handle*/
BEEPER_Tag Beeper0;

/*C4-B8的音调对应的频率大小*/
const uint16_t MusicNoteFrequency[] = {
    // rest_note
    0,
    // C    C#   D   Eb    E    F    F#   G    G#   A   Bb    B
    262,
    277,
    294,
    311,
    330,
    349,
    370,
    392,
    415,
    440,
    466,
    494,
    523,
    554,
    587,
    622,
    659,
    698,
    740,
    784,
    830,
    880,
    932,
    988,
    1047,
    1109,
    1175,
    1245,
    1319,
    1397,
    1480,
    1568,
    1661,
    1760,
    1865,
    1976,
    2093,
    2218,
    2349,
    2489,
    2637,
    2794,
    2960,
    3136,
    3322,
    3520,
    3729,
    3951,
    4186,
    4435,
    4699,
    4978,
    5274,
    5588,
    5920,
    6272,
    6645,
    7040,
    7459,
    7902,
    // check_note
    0,
};

/*全局TONE结构体指针，用于定时器TIM4中断函数中*/
TONE *MySound;

/*乐曲*/
TONE const BEEPER_KEYPRESS[] = {
    {NOTE_C6, 7},
    {CHECK_NOTE, 0}, // 检查位
};

TONE const BEEPER_TRITONE[] = {
    {NOTE_B5, 7},
    {REST_NOTE, 2},
    {NOTE_D6, 6},
    {REST_NOTE, 2},
    {NOTE_F6, 6},
    {CHECK_NOTE, 0}, // 检查位
};

TONE const BEEPER_WARNING[] = {
    {NOTE_F4, 5},
    {REST_NOTE, 2},
    {NOTE_F4, 5},
    {CHECK_NOTE, 0}, // 检查位
};

TONE const BEEP1[] = {
    {NOTE_C5, 11},
    {REST_NOTE, 2}, // 休止符
    {NOTE_C5, 11},
    {REST_NOTE, 2},
    {NOTE_G5, 11},
    {REST_NOTE, 2},
    {NOTE_G5, 11},
    {REST_NOTE, 2},
    {NOTE_A5, 11},
    {REST_NOTE, 2},
    {NOTE_A5, 11},
    {REST_NOTE, 2},
    {NOTE_G5, 22},
    {REST_NOTE, 2},
    {NOTE_F6, 11},
    {REST_NOTE, 2},
    {NOTE_F6, 11},
    {REST_NOTE, 2},
    {NOTE_E7, 11},
    {REST_NOTE, 2},
    {NOTE_E7, 11},
    {REST_NOTE, 2},
    {NOTE_D8, 11},
    {REST_NOTE, 2},
    {NOTE_D8, 11},
    {REST_NOTE, 2},
    {NOTE_C5, 11},
    {CHECK_NOTE, 0}, // 检查位
};

TONE const BEEP2[] = {
    {REST_NOTE, 20},
    {REST_NOTE, 20},
    {REST_NOTE, 20},
    {NOTE_C5, 10},
    {NOTE_B4, 10},

    {NOTE_A4, 20},
    {NOTE_E5, 40},
    {NOTE_C5, 10},
    {NOTE_A4, 10},

    {NOTE_B4, 20},
    {NOTE_F5, 20},
    {NOTE_E5, 10},
    {NOTE_D5, 30},

    {NOTE_C5, 10},
    {NOTE_D5, 10},
    {NOTE_C5, 10},
    {NOTE_D5, 10},
    {NOTE_E5, 20},
    {NOTE_C5, 10},
    {NOTE_B4, 10},

    {NOTE_A4, 20},
    {NOTE_D5, 20},
    {NOTE_C5, 10},
    {NOTE_B4, 10},
    {REST_NOTE, 10},
    {NOTE_A4, 5},
    {NOTE_B4, 5},

    {NOTE_C5, 20},
    {NOTE_A4, 20},
    {NOTE_E5, 20},
    {NOTE_C5, 20},

    {NOTE_D5, 20},
    {NOTE_A5, 20},
    {NOTE_G5, 20},
    {NOTE_F5, 10},
    {NOTE_E5, 5},
    {NOTE_D5, 5},

    {NOTE_E5, 80},

    {CHECK_NOTE, 0}, // 检查位
};

void TIM9_PWM_Init(uint16_t arr)
{
    /*初始化结构体*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /*开启rcc时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

    /*初始化TIM9*/
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = arr - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 84 - 1; // 使得频率剩下1000kHz
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseInitStructure);

    /*TIM9配置PWM模式*/
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = arr / 2; // 占空比
    TIM_OC2Init(TIM9, &TIM_OCInitStructure);

    TIM_CtrlPWMOutputs(TIM9, ENABLE);

    TIM_Cmd(TIM9, DISABLE); // 先关掉防止蜂鸣器怪叫
}

void Beeper_Init(void)
{
    /*初始化结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启rcc时钟*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /*初始化PA3-TIM9_CH2*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM9);

    /*初始化定时器TIM9*/
    TIM9_PWM_Init(1000);
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    /* BEEPER使能标志位 */
    Beeper0.Beeper_Enable = 1;
    Beeper0.Beeper_Continue_Flag = 0;

    Beeper0.Sound_Loud = 20;
}

/*计算对应的预重装值ARR（psc = 10 - 1）//用1000kHz / 音调频率*/
uint16_t Set_Musical_Note(uint16_t frq)
{
    /*防止休止符时蜂鸣器怪叫*/
    if (frq == 0)
        return 0;
    float temp = 0;
    temp = 1000000.0f / (float)frq;
    return (uint16_t)temp;
}

void Beeper_Set_Musical_Tone(uint16_t frq)
{
    float temp = 0;
    if (!(TIM9->CR1 & TIM_CR1_CEN))
    {
        TIM_Cmd(TIM9, ENABLE);
    }
    temp = 1000000.0f / (float)frq;
    TIM9->ARR = temp;
}

/**
 * @brief Beeper的应用层函数
 * @param  TONE *Sound 传入结构体数组
 * @retval 无
 */
void Beeper_Perform(const TONE *Sound)
{
    /*该变量用于计算结构体数组的长度*/
    uint16_t Note_Length;
    TIM_Cmd(TIM9, DISABLE);

    /*让全局结构体指针指向传入的乐曲*/
    MySound = (TONE *)Sound;

    /*通过寻找检查位CHECK_NOTE来计算传入的结构体长度//因为sizeof是在编译中完成的所以这里没法用*/
    for (Note_Length = 0; MySound[Note_Length].Note != CHECK_NOTE; Note_Length++)
        ;

    /*赋予长度大小*/
    Beeper0.Sound_Size = Note_Length;
    /*把音符表清零*/
    Beeper0.Beep_Play_Schedule = 0;

    /*开启蜂鸣器继续标志位*/
    Beeper0.Beeper_Continue_Flag = 1;
    Beeper0.Beeper_Count = 0;
}

/* 用于10ms定时器中断进行循环 */
void Beeper_Proc(void)
{
    /*判断是否继续*/
    if (Beeper0.Beeper_Continue_Flag && Beeper0.Beeper_Enable)
    {
        /*判断音符表走完没*/
        if (Beeper0.Beep_Play_Schedule <= Beeper0.Sound_Size)
        {
            /*时间减短10ms*/
            Beeper0.Beeper_Count--;
            /*这个操作的意思是如果count = 65535时，意思就是延时结束了，这个音符演完了*/
            if (!(Beeper0.Beeper_Count < 65535))
            {
                // printf("Ps:%d ", Beeper0.Beep_Play_Schedule);
                /*给预重装载值赋值，改变音调*/
                TIM9->ARR = (uint16_t)Set_Musical_Note(MusicNoteFrequency[MySound[Beeper0.Beep_Play_Schedule].Note]);
                /*给PWM占空比赋值，改变音量*/
                TIM9->CCR2 = (uint16_t)TIM9->ARR / (100 - Beeper0.Sound_Loud);
                /*赋值新的延时长度给count*/
                Beeper0.Beeper_Count = MySound[Beeper0.Beep_Play_Schedule].Delay;
                /*音符表走到下一个音符*/
                Beeper0.Beep_Play_Schedule++;
                TIM_Cmd(TIM9, ENABLE);
            }
        }
        /*失能蜂鸣器，清空标志位*/
        else
        {
            TIM_Cmd(TIM9, DISABLE);
            Beeper0.Beeper_Continue_Flag = 0;
            GPIO_ResetBits(GPIOA, GPIO_Pin_3);
        }
    }
    else
    {
        TIM_Cmd(TIM9, DISABLE);
        Beeper0.Beeper_Continue_Flag = 0;
        GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    }
}
