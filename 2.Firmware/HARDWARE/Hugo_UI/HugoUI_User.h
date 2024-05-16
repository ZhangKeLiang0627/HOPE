#ifndef __HUGOUI_USER_H
#define __HUGOUI_USER_H

#include "oled.h"
#include "OLED_User.h"
#include "Hugo_UI.h"
#include "stm32f4xx.h" // Device header

/* 声明全局变量 */
extern uint8_t *Switch_space[];

/**
 *    @变量组 *Slide_space[]
 *    @brief 滑动条控件 用于存储滑动条控件的值
 *    @param none
 *
 *    @注册
 *        -0->屏幕亮度
 *        -1->单选框测试
 *        -2->选项条固定长度
 */
enum Slide_space_Obj
{
    /* ---------- 屏幕亮度变量 ---------- */
    Slide_space_ScreenBrightness = 0,
    /* ---------- 动画速度变量 ---------- */
    Slide_space_Fre_x_speed,
    Slide_space_Fre_y_speed,
    Slide_space_Fre_width_speed,
    Slide_space_Slidbar_y_speed,
    Slide_space_Page_y_forlist_speed,
    Slide_space_Page_x_foricon_speed,

    /* ---------- 用户设置变量 ---------- */
    Slide_space_WS2812_R,
    Slide_space_WS2812_G,
    Slide_space_WS2812_B,
    Slide_space_val1,
    Slide_space_val2,
    Slide_space_val3,

    Slide_space_Volume_Ctrl,

    Slide_space_Obj_Max, // 标记成最大值

};

/**
 *    @变量组 *Switch_space[]
 *    @brief 开关控件 用于存储开关控件的值
 *    @param none
 *
 *    @注册
 *        -0->平滑动画
 *        -1->单选框测试
 *        -2->选项条固定长度
 */
enum Switch_space_Obj
{
    SwitchSpace_SmoothAnimation = 0, // 缓动动画flag需要在HugoUI_Control中使用，建议不要改动！（don't change this one!!!）

    SwitchSpace_OptionStripFixedLength,
    SwitchSpace_OledInverseColor,
    BeeperEnableConfig,
    PageOnlyListModeConfig,
    SwitchSpace_SetScreenFlip,

};

void HugoUI_InitLayout(void);
void HugoUIAnimationInit(void);

/* Event */
void EventUpdateOledLightLevel(void);
void EventBeeperEnableConfig(void);
void EventBeeperVolumeControl(void);
void EventOledSetScreenFlipMode(void);
void EventWS2812ColorControl(void);

void EventShowGyroUI(void);
void EventShowAboutUI(void);

void EventTreasureBox(void);
void EventChartTest(void);
void EventBlueTeeth(void);

void EventMagnetometer(void);
void EventShowCompassUI(void);
void EventRealTime(void);
void EventUSBConnet(void);
void EventBMP280Test(void);

void EventScanMyFiles(void);
void EventRC522(void);

#endif
