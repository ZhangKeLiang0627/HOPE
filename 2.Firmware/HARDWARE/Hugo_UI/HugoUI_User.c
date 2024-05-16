#include "HugoUI_User.h"
#include "Hugo_UI.h"
#include "HugoUI_BMP.h"
#include "u8g2.h"
#include "u8g2_Init.h"
#include "OLED_User.h"
#include "malloc.h"
#include "oled.h"
#include "BEEPER.h"
#include "usart.h"

#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

#include "FileSystem.h"
#include "WS2812B.h"

#include "myiic2.h"
#include "QMC5883L.h"
#include "led.h"
#include "sht30.h"
#include "PCF8563.h"
#include "fattester.h"
/******************************变量存放区******************************/

// ui控制变量
extern uint8_t ui_Key_num, ui_Encoder_num; // user可以将自己的实现函数的变量直接赋值给这两个Num

// 开关控件变量
uint8_t SmoothAnimation_Flag = true; // 缓动动画标志位 // 缓动动画flag需要在HugoUI_Control中使用，建议不要改动！（don't change this one!!!）
uint8_t OptionStripFixedLength_Flag = false;

uint8_t InverseColor_flag = false; // oled屏幕反色（白色）标志位

uint8_t BeeperEnable_flag = true;
uint8_t PageListMode_flag = false;
uint8_t SetScreenFlip_flag = false;

// 滑动条变量
float frame_y_speed = 65;
float frame_x_speed = 65;
float frame_width_speed = 65;
float slidbar_y_speed = 65;
float page_x_foricon_speed = 65;
float page_y_forlist_speed = 65;

paramType WS2812_R = 10.0f;
paramType WS2812_G = 10.0f;
paramType WS2812_B = 10.0f;

paramType val1 = 0.1f;
paramType val2 = 0.2f;
paramType val3 = 0.3f;

paramType BeeperLoud = 0;

paramType ScreenBrightness = 255;
paramType MenuScroll = 0;

float pitch, roll, yaw;
short aacx, aacy, aacz;
short gyrox, gyroy, gyroz;

extern float icon_move_x;
// extern u8g2_t u8g2;
u8g2_t u8g2; // 初始化u8g2结构体

/* -------------------- 结构体存放区 -------------------- */

// 由于sizeof的原理是在编译阶段由编译器替换数组大小，因此我们无法计算指针的sizeof！需要在位图的第一个字节填写 n 阶矩阵图像

uint8_t *Switch_space[] = {
    &SmoothAnimation_Flag, // 缓动动画flag需要在HugoUI_Control中使用，建议不要改动！（don't change this one!!!）
    &OptionStripFixedLength_Flag,
    &InverseColor_flag,
    &Beeper0.Beeper_Enable, // 蜂鸣器使能标志位
    &PageListMode_flag,
    &SetScreenFlip_flag,

};

/**
 *    @结构体 Slide_Bar Slide_space[]
 *    @brief 滑动条控件 用于存储滑动条控件的值(即储存param)
 *    @param none
 *
 *    @变量
 *        int   val    值
 *        int   min  最小值
 *        int   max  最大值
 *        int   step 步进
 */

struct Slide_Bar Slide_space[] = {
    /* ---------- 屏幕亮度变量 ---------- */
    {(float *)&ScreenBrightness, 1, 255, 16}, // 亮度调整

    /* ---------- 动画速度变量 ---------- */
    {(float *)&frame_x_speed, 20, 200, 1},
    {(float *)&frame_y_speed, 20, 200, 1},
    {(float *)&frame_width_speed, 20, 200, 1},
    {(float *)&slidbar_y_speed, 20, 200, 1},
    {(float *)&page_y_forlist_speed, 20, 200, 1},
    {(float *)&page_x_foricon_speed, 20, 200, 1},

    /* ---------- 用户设置变量 ---------- */
    {(float *)&WS2812_R, 0, 255, 1}, // WS2812
    {(float *)&WS2812_G, 0, 255, 1},
    {(float *)&WS2812_B, 0, 255, 1},

    {(float *)&val1, 0, 10, 0.1}, // param
    {(float *)&val2, 0, 10, 0.5},
    {(float *)&val3, 0, 10, 0.1},

    {(float *)&BeeperLoud, 0, 88, 5}, // 蜂鸣器响度

};

/* -------------------- 用户函数 -------------------- */

void HugoUI_InitLayout(void)
{
    /* 注册 Page */
    HugoUIPage_t *pageMain = AddPage(PAGE_CUSTOM, "pageMain");
    HugoUIPage_t *pageSetting = AddPage(PAGE_LIST, "pageSetting");
    HugoUIPage_t *pagePID = AddPage(PAGE_LIST, "pagePID");
    HugoUIPage_t *pageAnimation = AddPage(PAGE_LIST, "pageAnimation");
    HugoUIPage_t *pageWS2812 = AddPage(PAGE_LIST, "pageWS2812");
    HugoUIPage_t *pageFiles = AddPage(PAGE_LIST, "pageFiles")
                                  ->SetPgaeFunCallBack(HugoUIPageFilesCallBack);

    /* 注册 Item */
    // PageMain
    pageMain->AddItem(pageMain, "Setting", ITEM_JUMP_PAGE)
        ->SetJumpId(pageSetting->pageId, 0)
        ->SetIconSrc(Setting_BMP);

    pageMain->AddItem(pageMain, "Compass", ITEM_CALL_FUNCTION, EventShowCompassUI)
        ->SetIconSrc(Poet_BMP);

    pageMain->AddItem(pageMain, "MAG", ITEM_CALL_FUNCTION, EventMagnetometer)
        ->SetIconSrc(TreasureBox_BMP);

    pageMain->AddItem(pageMain, "Animation", ITEM_JUMP_PAGE)
        ->SetJumpId(pageAnimation->pageId, 0)
        ->SetIconSrc(Lighting_BMP);

    pageMain->AddItem(pageMain, "BMP280", ITEM_CALL_FUNCTION, EventBMP280Test)
        ->SetIconSrc(Poet_BMP);

    pageMain->AddItem(pageMain, "USB", ITEM_CALL_FUNCTION, EventUSBConnet)
        ->SetIconSrc(Robot_BMP);

    pageMain->AddItem(pageMain, "Gyro", ITEM_CALL_FUNCTION, EventShowGyroUI)
        ->SetIconSrc(Poet_BMP);

    pageMain->AddItem(pageMain, "BlueTeeth", ITEM_CALL_FUNCTION, EventBlueTeeth)
        ->SetIconSrc(Knife_BMP);

    pageMain->AddItem(pageMain, "RealTime", ITEM_CALL_FUNCTION, EventRealTime)
        ->SetIconSrc(Cuteghost_BMP);

    pageMain->AddItem(pageMain, "RGB", ITEM_JUMP_PAGE)
        ->SetJumpId(pageWS2812->pageId, 0)
        ->SetIconSrc(Unicorn_BMP);

    pageMain->AddItem(pageMain, "???", ITEM_CALL_FUNCTION, EventTreasureBox)
        ->SetIconSrc(TreasureBox_BMP);

    pageMain->AddItem(pageMain, "MyFiles", ITEM_JUMP_PAGE)
        ->SetJumpId(pageFiles->pageId, 0)
        ->SetIconSrc(TreasureBox_BMP);

    pageMain->AddItem(pageMain, "RC522", ITEM_CALL_FUNCTION, EventRC522)
        ->SetIconSrc(Cuteghost_BMP);

    pageMain->AddItem(pageMain, "About", ITEM_CALL_FUNCTION, EventShowAboutUI)
        ->SetIconSrc(Home_BMP);

    // PageSetting
    pageSetting->AddItem(pageSetting, "Setting", ITEM_PAGE_DESCRIPTION);
    pageSetting->AddItem(pageSetting, "PID Editor", ITEM_JUMP_PAGE)
        ->SetJumpId(pagePID->pageId, 0);

    pageSetting->AddItem(pageSetting, "InverseColor", ITEM_SWITCH, &InverseColor_flag, Oled_EventOledInverseColor);
    pageSetting->AddItem(pageSetting, "FilpScreen", ITEM_SWITCH, Switch_space[SwitchSpace_SetScreenFlip], EventOledSetScreenFlipMode);
    pageSetting->AddItem(pageSetting, "LightLevel", ITEM_CHANGE_VALUE, &ScreenBrightness, Oled_EventUpdateOledLightLevel);
    pageSetting->AddItem(pageSetting, "BeepEnable", ITEM_SWITCH, &BeeperEnable_flag, EventBeeperEnableConfig);
    pageSetting->AddItem(pageSetting, "UASRT62Printf", ITEM_SWITCH, &fputc_select_flag, NULL);
    pageSetting->AddItem(pageSetting, "Page2List", ITEM_SWITCH, &PageListMode_flag, NULL);
    pageSetting->AddItem(pageSetting, "Volume Ctrl", ITEM_CHANGE_VALUE, Slide_space[Slide_space_Volume_Ctrl].val, EventBeeperVolumeControl);
    pageSetting->AddItem(pageSetting, "SaveAll", ITEM_CALL_FUNCTION, EventSaveSettingConfig);

    pageSetting->AddItem(pageSetting, "Exit", ITEM_JUMP_PAGE)
        ->SetJumpId(pageMain->pageId, 0);

    // PageAnimation
    pageAnimation->AddItem(pageAnimation, "Animation", ITEM_PAGE_DESCRIPTION);
    pageAnimation->AddItem(pageAnimation, "SmoothAnim", ITEM_SWITCH, &SmoothAnimation_Flag, NULL);
    pageAnimation->AddItem(pageAnimation, "-PlsSetSpeed--", ITEM_PAGE_DESCRIPTION);
    pageAnimation->AddItem(pageAnimation, "Fre_x", ITEM_CHANGE_VALUE, &frame_x_speed, NULL);
    pageAnimation->AddItem(pageAnimation, "Fre_y", ITEM_CHANGE_VALUE, &frame_y_speed, NULL);
    pageAnimation->AddItem(pageAnimation, "Fre_width", ITEM_CHANGE_VALUE, &frame_width_speed, NULL);
    pageAnimation->AddItem(pageAnimation, "Slidbar_y", ITEM_CHANGE_VALUE, &slidbar_y_speed, NULL);
    pageAnimation->AddItem(pageAnimation, "list_y", ITEM_CHANGE_VALUE, &page_y_forlist_speed, NULL);
    pageAnimation->AddItem(pageAnimation, "icon_x", ITEM_CHANGE_VALUE, &page_x_foricon_speed, NULL);
    pageAnimation->AddItem(pageAnimation, "Exit", ITEM_JUMP_PAGE)
        ->SetJumpId(pageMain->pageId, 3);

    // PagePID
    pagePID->AddItem(pagePID, "PID Editor", ITEM_PAGE_DESCRIPTION);
    pagePID->AddItem(pagePID, "PID_Kp", ITEM_CHANGE_VALUE, &val1, NULL);
    pagePID->AddItem(pagePID, "PID_Ki", ITEM_CHANGE_VALUE, &val2, NULL);
    pagePID->AddItem(pagePID, "PID_Kd", ITEM_CHANGE_VALUE, &val3, NULL);
    pagePID->AddItem(pagePID, "Exit", ITEM_JUMP_PAGE)
        ->SetJumpId(pageSetting->pageId, 1);

    // PageWS2812
    pageWS2812->AddItem(pageWS2812, "WS2812RGBSet", ITEM_PAGE_DESCRIPTION);
    pageWS2812->AddItem(pageWS2812, "SetRed", ITEM_CHANGE_VALUE, &WS2812_R, EventWS2812ColorControl);
    pageWS2812->AddItem(pageWS2812, "SetGreen", ITEM_CHANGE_VALUE, &WS2812_G, EventWS2812ColorControl);
    pageWS2812->AddItem(pageWS2812, "SetBlue", ITEM_CHANGE_VALUE, &WS2812_B, EventWS2812ColorControl);
    pageWS2812->AddItem(pageWS2812, "Exit", ITEM_JUMP_PAGE)
        ->SetJumpId(pageMain->pageId, 9);

    HugoUIPageFilesAddItems(pageFiles, "1:");
    // ItemTail = ItemHead;
    // while(ItemTail != NULL)
    // {
    //     printf("name:%s\r\n", ItemTail->title);
    //     printf("lid:%d\r\n", ItemTail->lineId);
    //     printf("id:%d\r\n", ItemTail->itemId);
    //     ItemTail = ItemTail->next;
    // }
    // pageTail = pageHead;
    // while(pageTail != NULL)
    // {
    //     printf("name:%s\r\n", pageTail->title);
    //     printf("id:%d\r\n", pageTail->pageId);
    //     pageTail = pageTail->next;
    // }

    printf("malloc used:%d\r\n", my_mem_perused(SRAMIN));
    printf("itemMax:%d\r\n", pageSetting->itemMax);
}

/* Little Event */
/* 对于SSD1306型号的Oled的调节屏幕亮度的函数 */
void EventUpdateOledLightLevel(void)
{
    WriteCmd(0x81);                                                    /*contrast control*/
    WriteCmd((uint8_t)*Slide_space[Slide_space_ScreenBrightness].val); /*255*/
}

/* Oled设置屏幕翻转的函数 */
void EventOledSetScreenFlipMode(void)
{
    if (*Switch_space[SwitchSpace_SetScreenFlip] == true)
    {
        Oled_u8g2_SetFlipMode(2);
    }
    else
    {
        Oled_u8g2_SetFlipMode(0);
    }
}

/* 蜂鸣器使能事件函数 */
void EventBeeperEnableConfig(void)
{
    Beeper0.Beeper_Enable = BeeperEnable_flag;
}

/* 蜂鸣器音量控制事件函数 */
void EventBeeperVolumeControl(void)
{
    Beeper0.Sound_Loud = (uint8_t)BeeperLoud;
}

/* RGB灯珠颜色控制事件函数 */
void EventWS2812ColorControl(void)
{
    WS2812B_Write_24Bits(1, (uint32_t)((uint16_t)WS2812_G << 16 | (uint16_t)WS2812_R << 8 | (uint16_t)WS2812_B));
    WS2812B_Show();
}

/* Large Event */

extern uint8_t mpu6050_set_flag;
extern uint8_t sht30_set_flag;

float Temperature, Humidity;

/* 陀螺仪事件函数 */
void EventShowGyroUI(void)
{
    static uint8_t GyroInit_flag1 = 1, GyroInit_flag2 = 1;
    static float GyroFrameBox_Width = 0, GyroFrameBox_Width_trg = 0;
    static int len;
    static uint8_t *p;

    len = 8 * Oled_u8g2_GetBufferTilHeight() * Oled_u8g2_GetBufferTileWidth();
    p = Oled_u8g2_GetBufferPtr();

    Oled_u8g2_ShowStr(0, 13, "IMU:");
    if (!GyroInit_flag1 && !GyroInit_flag2)
    {
        Oled_u8g2_SetDrawColor(2);
        Oled_u8g2_DrawBox(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
        Oled_u8g2_SetDrawColor(1);

        for (uint16_t i = len / 2; i < len; i++)
        {
            if (i % 2 == 0)
            {
                p[i] = p[i] & (0x55);
                p[i] = p[i] & (0x00);
            }
            else
                p[i] = p[i] & (0xaa);
        }
        Oled_u8g2_ShowStr(100, 13, "OK");
    }
    else
    {
        Oled_u8g2_ShowStr(100, 13, "ERR");
        Oled_u8g2_SetFont(u8g2_font_wqy13_t_gb2312a);
        Oled_u8g2_DrawRFrame(128 - (GyroFrameBox_Width + 7), 33, GyroFrameBox_Width, FONT_HEIGHT, 1);
        Oled_u8g2_ShowUTF8((128 - Oled_u8g2_Get_UTF8_ASCII_PixLen(" 请等待初始化完成 ")) / 2, 48, " 请等待初始化完成 "); // 请等待初始化完成

        Oled_u8g2_SetDrawColor(2);
        Oled_u8g2_DrawRBox(GyroFrameBox_Width / 8, 35, GyroFrameBox_Width, FONT_HEIGHT, 0);
        Oled_u8g2_SetDrawColor(1);
        if (GyroFrameBox_Width_trg != 100)
            GyroFrameBox_Width_trg = 105;
        // HugoUISendBuffer();
        // Oled_u8g2_SetFont(u8g2_font_profont15_mr);
    }

    if (HugoUI_Animation_Linear(&GyroFrameBox_Width, &GyroFrameBox_Width_trg, 85) == 0)
    {
        if (GyroInit_flag1 == 1)
        {
            GyroInit_flag1 = MPU_Init();
            printf("MPU_Init = %d\r\n", GyroInit_flag1);
            HugoUISendBuffer();
        }
        if (GyroInit_flag2 == 1)
        {
            uint16_t InitTimeout = 0;
            while (GyroInit_flag2)
            {
                GyroInit_flag2 = mpu_dmp_init();

                InitTimeout++;
                if (InitTimeout > 10000)
                    break;
            }
            printf("mpu_dmp_Init = %d\r\n", GyroInit_flag2);
        }
    }

    if (HugoUIExecuteRate(&Rate100Hz))
    {
        if (!GyroInit_flag1 && !GyroInit_flag2)
        {
            mpu_dmp_get_data(&pitch, &roll, &yaw);
            printf("pitch = %.1f ", pitch);
            printf("roll = %.1f ", roll);
            printf("yaw = %.1f\r\n", yaw);
            if (pitch > 10 || pitch < -10)
            {
                GyroFrameBox_Width_trg = 84;
            }
            else
                GyroFrameBox_Width_trg = 0;
        }
    }

    Oled_u8g2_ShowFloat(50, 13, roll, 2, 2);
    Oled_u8g2_ShowFloat(50, 26, yaw, 2, 2);
    Oled_u8g2_ShowFloat(0, 26, pitch, 2, 2);

    Oled_u8g2_SetDrawColor(2);
    Oled_u8g2_DrawBox(0, 0, GyroFrameBox_Width, 32);
    Oled_u8g2_DrawTriangle(GyroFrameBox_Width, 0, GyroFrameBox_Width, 32, GyroFrameBox_Width > 24 ? GyroFrameBox_Width + 24 : GyroFrameBox_Width, 32);
    Oled_u8g2_SetDrawColor(1);

    // HugoUI_Animation_Linear(&GyroFrameBox_Width, &GyroFrameBox_Width_trg, 60);

    if (ui_Key_num == 2)
    {
        icon_move_x = 0; // 倘若跳转到custom界面可以起到一个复位动效
    }
}

/* About的应用事件函数 */
void EventShowAboutUI(void)
{
    static float motion_a = 80.0f, motion_a_trg = 0;
    // Oled_SetOledInverseColor(1);
    Oled_u8g2_ShowBMP(motion_a, 0, 40, 50, HeadSculpture_BMP);
    Oled_u8g2_ShowStr(50, FONT_HEIGHT, "HOPE  Pro");
    Oled_u8g2_ShowUTF8(45, FONT_HEIGHT * 2, "版本: Ver1.3");
    Oled_u8g2_ShowUTF8(45, FONT_HEIGHT * 3, "储存: 16 MB");
    Oled_u8g2_ShowUTF8(10, FONT_HEIGHT * 4, " By @kkl_aka科良");
    Oled_u8g2_SetDrawColor(2);
    Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    Oled_u8g2_DrawRBox(86, 3, 30, FONT_HEIGHT, 0);
    Oled_u8g2_SetDrawColor(1);

    HugoUI_Animation_Linear(&motion_a, &motion_a_trg, 85);

    if (ui_Key_num == 2)
    {
        // if (*Switch_space[SwitchSpace_OledInverseColor] == false)
        // {
        //     Oled_SetOledInverseColor(0);
        // }
        motion_a = 80.0f;
        icon_move_x = 128; // 倘若跳转到custom界面可以起到一个复位动效
    }
}

/* TreasureBox的事件函数 */
void EventTreasureBox(void)
{
    Oled_DrawIntensiveComputing();
}

// width: 16, height: 16
/* 波形绘制的测试事件函数 */
void EventChartTest(void)
{ // Oled_DrawIntensiveComputing();
    static float a = 0, a_trg = 64;
    if (a == a_trg)
    {
        if (a == 0)
        {
            a_trg = 64;
        }
        else if (a == 64)
            a_trg = 0;
    }
    HugoUI_Animation_Linear(&a, &a_trg, 30);

    // Oled_DrawSlowBitmapResize(80 - a / 2, 16 - a / 4, LittleHreat_BMP, 16, 16, a / 1.5f, a / 1.5f);

    // Oled_DrawSlowBitmapResize(32 - a / 2, 32 - a / 4, Poet_BMP, 32, 32, 42, 42);
    // u8g2_SetBitmapMode(&u8g2, 0);
    // u8g2_DrawXBMP(&u8g2, 100, 0, 16, 16, LittleHreat_BMP);

    // u8g2_SetBitmapMode(&u8g2, 1);

    u8g2_DrawXBMP(&u8g2, 50, 12, 32, 32, Lighting_BMP);

    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawXBMP(&u8g2, 50, 20, 32, 32, Poet_BMP);
    u8g2_SetDrawColor(&u8g2, 1);

    // u8g2_SetBitmapMode(&u8g2, 0);

    if (ui_Key_num == 2)
    {
        a = 0;
        icon_move_x = 128; // 倘若跳转到custom界面可以起到一个复位动效
    }
}

void BlueTeethSerialCmd(void)
{
    if ((USART6_RX_STA & 0x8000) != 0)
    {
        switch (USART6_RX_BUF[0])
        {
        /* H\r\n (Hello)*/
        case 'H':
            USART6_SendBuf("Hello HOPE!\r\n");
            USART6_SendBuf("   __  __    ___    ____    ____\r\n");
            USART6_SendBuf("  / /_/ / / __  / / --- / /____/\r\n");
            USART6_SendBuf(" / __  / / /_/ / / ---_/ /___/\r\n");
            USART6_SendBuf("/ / /_/ / ___ / /_/     /_____/\r\n");

            break;

        /* L xx xx xx\r\n (Light)*/
        case 'L':
            WS2812B_Write_24Bits(1, (uint32_t)((uint16_t)atoi((const char *)USART6_RX_BUF + 4) << 16 | (uint16_t)atoi((const char *)USART6_RX_BUF + 1) << 8 | (uint16_t)atoi((const char *)USART6_RX_BUF + 7)));
            WS2812B_Show();
            break;

        case 'G':
            /* GT\r\n (GetTime)*/
            if (USART6_RX_BUF[1] == 'T')
            {
                MyIIC_Init();
                if (PCF8563_Init() == 0 && PCF8563_GetTime() == 0) // 如果读成功就发送
                {
                    char formatTimetemp[30];
                    sprintf(formatTimetemp, "%d.%d.%d %d:%d:%d\r\n", RTC_Time.year, RTC_Time.month, RTC_Time.date, RTC_Time.hour, RTC_Time.min, RTC_Time.sec);
                    USART6_SendBuf("当前时间:");
                    USART6_SendBuf((uint8_t *)formatTimetemp);
                }
                else
                {
                    USART6_SendBuf("Time is unavailable!时间无法获取!\r\n");
                }
            }

            /* GW\r\n (GetWeather)*/
            else if (USART6_RX_BUF[1] == 'W')
            {
                float tem, hum; // 定义温度和湿度的临时变量
                MyIIC_Init();
                if (!SHT30_Read_Value(&tem, &hum)) // 如果读到值就发送
                {
                    char formatTHtemp[40];
                    sprintf(formatTHtemp, "temperature:%.1f, humidity:%.1f\r\n", tem, hum);
                    USART6_SendBuf((uint8_t *)formatTHtemp);
                }
                else
                {
                    USART6_SendBuf("Weather is unavailable!温湿度无法获取!\r\n");
                }
            }
            break;

        case 'S':
            /* STxxxx xx xx xx xx xx\r\n (SetTime)*/
            if (USART6_RX_BUF[1] == 'T')
            {
                RTC_Time.year = atoi((const char *)USART6_RX_BUF + 2);
                RTC_Time.month = atoi((const char *)USART6_RX_BUF + 7);
                RTC_Time.date = atoi((const char *)USART6_RX_BUF + 10);
                RTC_Time.hour = atoi((const char *)USART6_RX_BUF + 13);
                RTC_Time.min = atoi((const char *)USART6_RX_BUF + 16);
                RTC_Time.sec = atoi((const char *)USART6_RX_BUF + 19);
                PCF8563_SetTime(RTC_Time.year, RTC_Time.month, RTC_Time.date, RTC_Time.hour, RTC_Time.min, RTC_Time.sec);
                if (PCF8563_Init() == 0 && PCF8563_GetTime() == 0) // 如果读成功就发送
                {
                    char formatTimetemp[30];
                    sprintf(formatTimetemp, "%d.%d.%d %d:%d:%d\r\n", RTC_Time.year, RTC_Time.month, RTC_Time.date, RTC_Time.hour, RTC_Time.min, RTC_Time.sec);
                    USART6_SendBuf("Set time OK!\r\n");
                    USART6_SendBuf((uint8_t *)formatTimetemp);
                }
                else
                {
                    USART6_SendBuf("Set Time fail!\r\n请按(年月日时分秒)格式\r\nxxxx xx xx xx xx xx\r\n");
                }
            }
            break;

        case 'C':
            /* CMD\r\n (通用指令查询)*/
            if (USART6_RX_BUF[1] == 'M' && USART6_RX_BUF[2] == 'D')
            {
                USART6_SendBuf("H(SayHi)\r\nGT(GetTime)\r\nGW(GetWeather)\r\nST(SetTime)\r\n");
                USART6_SendBuf("指令尾巴上要记得加回车噢!\r\n");
            }
            /* CAT\r\n (AT指令查询)*/
            else if (USART6_RX_BUF[1] == 'A' && USART6_RX_BUF[2] == 'T')
            {
                USART6_SendBuf("AT+xx\r\nBD(ChangeSPPName)\r\nTD(GetSPPName)\r\nBM(ChangeBLEName)\r\nTM(GetBLEName))\r\nTN(GetAddress)\r\nCZ(SoftReset)\r\n");
                USART6_SendBuf("指令尾巴上要记得加回车噢!\r\n");
                USART6_SendBuf("修改完记得发指令软复位噢!\r\n");
            }
            break;

        /* AT... (AT指令执行)*/
        case 'A':
            if (USART6_RX_BUF[1] == 'T')
            {
                USART6_SendBuf(USART6_RX_BUF);
                USART6_SendBuf("\r\n");
            }
            break;

        default:
            USART6_SendBuf(USART6_RX_BUF);
            USART6_SendBuf("->");
            USART6_SendBuf("Not Cmd!\r\n");
            break;
        }
        USART6_RX_STA = 0;
        memset(USART6_RX_BUF, 0, 32);
    }
}

static void EventBlueTeeth(void)
{
    static uint8_t BluteethInit_flag = 1;
    static uint8_t BluteethFuncEnter_flag = 1;
    static float img_a = 4, img_a_trg = 24;

    // 初始化
    if (BluteethInit_flag)
    {
        USART6_Init(115200);

        BluteethInit_flag = 0;
    }

    // 进入该Func时执行
    if (BluteethFuncEnter_flag)
    {
        Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (HugoUI_Animation_Blur() == 0)
            BluteethFuncEnter_flag = 0;
        return;
    }

    BlueTeethSerialCmd(); // 执行BLE-SPP指令动作

    // 判断蓝牙是否连接
    if (PCin(5) == 0)
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "ERR");
    }
    else
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "OK");
    }

    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT, "蓝牙SPP");
    Oled_u8g2_ShowStr(0, 2 * FONT_HEIGHT, "RX:");

    // 如果没有输入回车，内容显示在屏幕上
    Oled_u8g2_ShowUTF8(0, 3 * FONT_HEIGHT, (char *)USART6_RX_BUF);

    // img缩放动效
    if (img_a == img_a_trg)
    {
        if (img_a == 4)
        {
            img_a_trg = 24;
        }
        else if (img_a == 24)
            img_a_trg = 4;
    }
    HugoUI_Animation_EasyIn(&img_a, &img_a_trg, 100);

    Oled_DrawSlowBitmapResize(118 - img_a / 2, 50 - img_a / 4, Knife_BMP, 32, 32, img_a, img_a);

    // Event
    if (ui_Key_num == 1)
    {
        USART6_SendBuf("Hello");
    }
    else if (ui_Key_num == 2)
    {
        img_a = 4;
        BluteethFuncEnter_flag = 1;
        icon_move_x = 128; // 倘若跳转到custom界面可以起到一个复位动效
    }
}

/* 磁力计 */
void EventMagnetometer(void)
{
    static uint8_t EventMagnetometerInit_flag = 1;
    static uint8_t EventMagnetometerFuncEnter_flag = 1;
    static int mag[3];
    static uint8_t tmp[2];

    // 初始化
    if (EventMagnetometerInit_flag == 1)
    {
        MyIIC_Init();
        // QMC5883LSetMode(1);
        EventMagnetometerInit_flag = QMC5883LInit();

        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "ERR");
    }
    else
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "OK");
    }

    // Enter Func
    if (EventMagnetometerFuncEnter_flag)
    {
        Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (HugoUI_Animation_Blur() == 0)
            EventMagnetometerFuncEnter_flag = 0;
        return;
    }

    // Execute
    if (HugoUIExecuteRate(&Rate20Hz))
    {
        QMC5883LRead(mag);
        MyIIC_Read_MultiBytes(QMC5883L_IIC_ADDR, QMC5883L_TEMP_LSB, 2, tmp);
    }

    // View
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT, "MAG");
    Oled_u8g2_ShowNum(50, FONT_HEIGHT, ((uint16_t)tmp[1] << 8) | tmp[0]);
    Oled_u8g2_ShowFloat(0, 2 * FONT_HEIGHT, mag[0], 1, 1);
    Oled_u8g2_ShowFloat(0, 3 * FONT_HEIGHT, mag[1], 1, 1);
    Oled_u8g2_ShowFloat(0, 4 * FONT_HEIGHT, mag[2], 1, 1);

    if (ui_Key_num == 2)
    {
        EventMagnetometerFuncEnter_flag = 1;
    }
}

/* 磁力计 */
void EventShowCompassUI(void)
{
    static uint8_t EventShowCompassUIInit_flag = 1;
    static uint8_t EventShowCompassUIFuncEnter_flag = 1;
    static int mag[3];
    static float FrameBox_Width, FrameBox_Width_trg = 105;
    float temp;
    static float last_temp = 0;
    static int angle_from_north;
    static int cosA, sinA;
    // Enter Func
    if (EventShowCompassUIFuncEnter_flag)
    {
        Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (HugoUI_Animation_Blur() == 0)
            EventShowCompassUIFuncEnter_flag = 0;
        return;
    }
    if (EventShowCompassUIInit_flag == 1)
    {
        Oled_u8g2_DrawRFrame(128 - (FrameBox_Width + 7), 33, FrameBox_Width, FONT_HEIGHT, 1);
        Oled_u8g2_ShowUTF8((128 - Oled_u8g2_Get_UTF8_ASCII_PixLen(" 请等待初始化完成 ")) / 2, 48, " 请等待初始化完成 "); // 请等待初始化完成

        Oled_u8g2_SetDrawColor(2);
        Oled_u8g2_DrawRBox(FrameBox_Width / 8, 35, FrameBox_Width, FONT_HEIGHT, 0);
        Oled_u8g2_SetDrawColor(1);
        // if (GyroFrameBox_Width_trg != 105)
        //     GyroFrameBox_Width_trg = 105;
    }

    // Init
    if (HugoUI_Animation_Linear(&FrameBox_Width, &FrameBox_Width_trg, 75) == 0)
    {
        // 初始化
        if (EventShowCompassUIInit_flag == 1)
        {
            MyIIC_Init();
            // QMC5883LSetMode(1);
            EventShowCompassUIInit_flag = QMC5883LInit();
            if (EventShowCompassUIInit_flag)
            {
                printf("QMC5883L Err\r\n");
            }
            EventShowCompassUIInit_flag = mpu_dmp_init();
            if (EventShowCompassUIInit_flag)
            {
                printf("MPU6050 Err\r\n");
            }

            Oled_u8g2_ShowStr(100, FONT_HEIGHT, "ERR");
        }
        else
        {
            Oled_u8g2_ShowStr(100, FONT_HEIGHT, "OK");
        }
    }
    else
    {
        return;
    }

    // Execute Gyro
    if (HugoUIExecuteRate(&Rate100Hz))
    {
        mpu_dmp_get_data(&pitch, &roll, &yaw);
    }

    // Execute MAG
    if (HugoUIExecuteRate(&Rate50Hz))
    {
        QMC5883LRead(mag);
        temp = QMC5883LCalYaw(mag, pitch, roll);

        if ((temp > 360.0f) || (temp < 0.0f))
            temp = 360;
        if ((temp - last_temp < -250.0f) || (temp - last_temp > 250.0f))
            last_temp = temp;
        else
            last_temp = temp * 0.4f + last_temp * 0.6f;

        angle_from_north = last_temp + 0.5f;
        angle_from_north -= 90;
        if (angle_from_north < 0)
            angle_from_north += 360;

        cosA = cosf(angle_from_north * 0.017453292f) * 72;
        sinA = sinf(angle_from_north * 0.017453292f) * 72;
    }

    // View
    Oled_u8g2_ShowNum(0, FONT_HEIGHT, angle_from_north);
    Oled_u8g2_ShowUTF8(64 - sinA - 8, 96 - cosA - 8, " 北 ");
    Oled_u8g2_ShowUTF8(64 + cosA - 8, 96 - sinA - 8, " 东 ");
    Oled_u8g2_ShowUTF8(64 + sinA - 8, 96 + cosA - 8, " 南 ");
    Oled_u8g2_ShowUTF8(64 - cosA - 8, 96 + sinA - 8, " 西 ");

    if (ui_Key_num == 2)
    {
        EventShowCompassUIFuncEnter_flag = 1;
    }
}

/* 实时时钟测试 */
char formatRealTime[20];

void EventRealTime(void)
{
    static uint8_t EventRealTimeInit_flag = 1;
    static uint8_t EventRealTimeFuncEnter_flag = 1;

    // 初始化
    if (EventRealTimeInit_flag == 1)
    {
        MyIIC_Init();
        EventRealTimeInit_flag = PCF8563_Init();
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "ERR");
    }
    else
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "OK");
    }

    // Enter Func
    if (EventRealTimeFuncEnter_flag)
    {
        Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (HugoUI_Animation_Blur() == 0)
            EventRealTimeFuncEnter_flag = 0;
        return;
    }

    // Execute
    if (HugoUIExecuteRate(&Rate5Hz))
    {
        PCF8563_GetTime();
        sprintf(formatRealTime, "%d.%d.%d %d:%d:%d", RTC_Time.year, RTC_Time.month, RTC_Time.date, RTC_Time.hour, RTC_Time.min, RTC_Time.sec);
    }

    // View
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT, "当前时间");
    Oled_u8g2_ShowStr(5, 2 * FONT_HEIGHT, formatRealTime);
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT * 3, "今天是周六吗?");
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT * 4, RTC_Time.day == 6 ? "是的" : "不是");

    // Event
    if (ui_Key_num == 2)
    {
        EventRealTimeFuncEnter_flag = 1;
        icon_move_x = 128; // 倘若跳转到custom界面可以起到一个复位动效
    }
}

#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG; // USB状态
extern vu8 bDeviceState;   // USB连接 情况
u8 offline_cnt = 0;
u8 tct = 0;
u8 USB_STA;
u8 Divece_STA;
void EventUSBConnet(void)
{
    static uint8_t EventUSBConnetInit_flag = 1;
    static uint8_t EventUSBConnetFuncEnter_flag = 1;
    // Enter Func
    if (EventUSBConnetFuncEnter_flag)
    {
        Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (HugoUI_Animation_Blur() == 0)
            EventUSBConnetFuncEnter_flag = 0;
        return;
    }
    // Init
    if (EventUSBConnetInit_flag == 1) // 初始化USB
    {
        USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_MSC_cb, &USR_cb);
        delay_ms(100);
        EventUSBConnetInit_flag = 0;
    }

    if (Divece_STA != bDeviceState)
    {
        if (bDeviceState == 1)
            printf("USB Connected\r\n"); // 提示USB连接已经建立
        else
            printf("USB DisConnected\r\n"); // 提示USB被拔出了
        Divece_STA = bDeviceState;
    }

    if (HugoUIExecuteRate(&Rate5Hz))
    {
        LED0 = !LED0; // 提示系统在运行
        if (USB_STATUS_REG & 0x10)
        {
            offline_cnt = 0; // USB连接了,则清除offline计数器
            bDeviceState = 1;
        }
        else // 没有得到轮询
        {
            offline_cnt++;
            if (offline_cnt > 10)
                bDeviceState = 0; // 2s内没收到在线标记,代表USB被拔出了
        }
        USB_STATUS_REG = 0;
    }

    // view
    if (USB_STATUS_REG & 0x10)
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "OK");
    }
    else // 没有得到轮询
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "ERR");
    }

    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT, "USB连接");
    Oled_u8g2_ShowUTF8((128 - Oled_u8g2_Get_UTF8_ASCII_PixLen("挂载Flash为虚拟U盘!")) / 2, FONT_HEIGHT * 2.5f, "挂载Flash为虚拟U盘!");

    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT * 4, "状态:");

    if (USB_STATUS_REG & 0x01) // 正在写 // 提示USB正在写入数据
    {
        LED0 = 0;

        Oled_u8g2_ShowUTF8(40, FONT_HEIGHT * 4, "正在写入");

        printf("USB is Writing...\r\n");
    }
    else if (USB_STATUS_REG & 0x02) // 正在读 // 提示USB正在读出数据
    {
        LED0 = 0;

        Oled_u8g2_ShowUTF8(40, FONT_HEIGHT * 4, "正在读取");

        printf("USB is Reading...\r\n");
    }
    else
        Oled_u8g2_ShowUTF8(40, FONT_HEIGHT * 4, "没有操作");

    // Event
    if (ui_Key_num == 2)
    {
        EventUSBConnetFuncEnter_flag = 1;
        icon_move_x = 128; // 倘若跳转到custom界面可以起到一个复位动效
    }
}

#include "BMP280.h"
void EventBMP280Test(void)
{
    /*Parameter Configuration*/
    static uint8_t BMP280TestInit_flag = 1;
    static uint8_t BMP280TestFuncEnter_flag = 1;

    static double BMP_Pressure, BMP_Temperature;

    // Init
    if (BMP280TestInit_flag)
    {
        MyIIC_Init();
        BMP280TestInit_flag = BMP280_Init();

        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "ERR");
    }
    else
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "OK");
    }

    // Enter Func
    if (BMP280TestFuncEnter_flag)
    {
        Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (HugoUI_Animation_Blur() == 0)
            BMP280TestFuncEnter_flag = 0;
        return;
    }

    // Execute
    if (HugoUIExecuteRate(&Rate20Hz))
    {
        if (BMP280_GetStatus(BMP280_MEASURING) != RESET)
        {
            if (BMP280_GetStatus(BMP280_IM_UPDATE) != RESET)
            {
                BMP_Temperature = BMP280_GetTemperature_double();
                BMP_Pressure = BMP280_GetPressure_double();
            }
        }
    }

    // view
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT, "BMP280气压计 ");
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT * 2, "气压/Pa:");
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT * 3, "温度/C:");

    Oled_u8g2_ShowFloat(60, FONT_HEIGHT * 2, BMP_Pressure, 2, 3);
    Oled_u8g2_ShowFloat(60, FONT_HEIGHT * 3, BMP_Temperature, 2, 3);

    // Event
    if (ui_Key_num == 2)
    {
        BMP280TestFuncEnter_flag = 1;
        icon_move_x = 128; // 倘若跳转到custom界面可以起到一个复位动效
    }
}

void EventScanMyFiles(void)
{
    static uint8_t EventScanMyFilesEnter_flag = 1;

    // Enter Func
    if (EventScanMyFilesEnter_flag)
    {
        mf_scan_files("1:");
        EventScanMyFilesEnter_flag = 0;
    }

    // Event
    if (ui_Key_num == 2)
    {
        EventScanMyFilesEnter_flag = 1;
    }
}

#include "RC522.h"
/* RC522模块测试函数 */
void EventRC522(void)
{
    static uint8_t EventRC522Enter_flag = 1;
    static uint8_t EventRC522Init_flag = 1;
    static uint8_t EventRC522_mode = 0; // 0->读卡 / 1->写卡
    uint8_t str2hex_buf[15];
    static uint8_t data[16] = {0}; // UID卡信息

    // Init
    if (EventRC522Init_flag != 0)
    {
        EventRC522Init_flag = RC522_Init();

        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "ERR");
    }
    else
    {
        Oled_u8g2_ShowStr(100, FONT_HEIGHT, "OK");
    }

    // Enter
    if (EventRC522Enter_flag)
    {
        EventRC522Enter_flag = 0;
    }

    // Execute
    if (HugoUIExecuteRate(&Rate5Hz))
    {
        // RC522_SearchCard(data);
    }

    // view
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT, EventRC522_mode == 0 ? "RC522读卡" : "RC522写卡");
    sprintf(str2hex_buf, "%x %x %x %x\r\n", data[0], data[1], data[2], data[3]);
    Oled_u8g2_ShowUTF8(0, FONT_HEIGHT * 2, str2hex_buf);

    // Event

    if (ui_Key_num == 1)
    {
        (EventRC522_mode == 0) ? RC522_SearchCard(data) : RC522_WriteCard(data);
    }
    else if (ui_Encoder_num)
    {
        EventRC522_mode = !EventRC522_mode;
    }
    else if (ui_Key_num == 2)
    {
        EventRC522Enter_flag = 1;
    }
}

// 初始化HugoUI,同时播放开场动画
void HugoUIAnimationInit(void)
{
    uint8_t HugoUIEnter_flag = 1;
    uint8_t HugoUIInit_flag = 1;
    float FrameBox_Width, FrameBox_Width_trg = 105;

    u8g2Init(&u8g2);                                // u8g2图形库初始化
    u8g2_SetFont(&u8g2, u8g2_font_wqy13_t_gb2312a); // 选择字库 // 内存不够就用u8g2_font_profont15_mr

    while (1)
    {
        Oled_u8g2_ClearBuffer();
        // Enter Func
        if (HugoUIEnter_flag)
        {
            Oled_u8g2_DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            if (HugoUI_Animation_Blur() == 0)
                HugoUIEnter_flag = 0;
            Oled_u8g2_SendBuffer();
            continue;
        }
        if (HugoUIInit_flag == 1)
        {
            Oled_u8g2_DrawRFrame(128 - (FrameBox_Width + 7), 33 - 10, FrameBox_Width, FONT_HEIGHT, 1);
            Oled_u8g2_ShowUTF8((128 - Oled_u8g2_Get_UTF8_ASCII_PixLen(" 请等待初始化完成 ")) / 2, 48 - 10, " 请等待初始化完成 "); // 请等待初始化完成
            Oled_u8g2_ShowUTF8((128 - Oled_u8g2_Get_UTF8_ASCII_PixLen("HelloHOPE")) / 2, FONT_HEIGHT, "HelloHOPE");
            Oled_u8g2_SetDrawColor(2);
            Oled_u8g2_DrawRBox(FrameBox_Width / 8, 35 - 10, FrameBox_Width, FONT_HEIGHT, 0);
            Oled_u8g2_SetDrawColor(1);
        }

        // Init
        if (HugoUI_Animation_Linear(&FrameBox_Width, &FrameBox_Width_trg, 95) == 0 && HugoUIInit_flag == 1)
        {

            HugoUI_InitLayout();
            HugoUIInit_flag = 0;
            break;
        }
        Oled_u8g2_SendBuffer();
    }
}
