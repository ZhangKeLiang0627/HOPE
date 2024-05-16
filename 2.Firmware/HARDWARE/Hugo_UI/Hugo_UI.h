#ifndef __HUGO_UI_H
#define __HUGO_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*User_Add*/
/*解决关键字不识别问题*/
#include "stm32f4xx.h" // Device header
#include "delay.h"
#include "oled.h"
#include "OLED_User.h"
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

/* 定义参数 */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define FONT_WIDTH 8
#define FONT_HEIGHT 15
#define SCROLL_BAR_WIDTH 3

/* USER_NEED_TO_FILL_THIESE_FUNCTION */
#define HugoUIScreenInit() (OLED_Init())
#define HugoUIDisplayStr(x, y, str) (Oled_u8g2_ShowStr(x, y, str))
#define HugoUIDisplayFloat(x, y, dat, num, pointNum) (Oled_u8g2_ShowFloat(x, y, dat, num, pointNum))
#define HugoUIDrawDot(x, y) (Oled_u8g2_DrawPoint(x, y))
#define HugoUIDrawBox(x, y, width, height) (Oled_u8g2_DrawBox(x, y, width, height))
#define HugoUIDrawFrame(x, y, width, height) (Oled_u8g2_DrawFrame(x, y, width, height))
#define HugoUIDrawRFrame(x, y, width, height, r) (Oled_u8g2_DrawRFrame(x, y, width, height, r))
#define HugoUIDrawRBox(x, y, width, height, r) (Oled_u8g2_DrawRBox(x, y, width, height, r))
#define HugoUIClearBuffer() (Oled_u8g2_ClearBuffer())
#define HugoUISendBuffer() (Oled_u8g2_SendBuffer())
#define HugoUISetDrawColor(mode) (Oled_u8g2_SetDrawColor(mode))
#define HugoUIDisplayBMP(x, y, width, height, pic) (Oled_u8g2_ShowBMP(x, y, width, height, pic))
#define HugoUIModifyColor(mode) (Oled_u8g2_ModifyColor(mode))
#define HugoUIShowUTF8(x, y, str) (Oled_u8g2_ShowUTF8(x, y, str))

#ifdef FPU
    // typedef     double      paramType;
    typedef float paramType; // 先使用float
#else
typedef int32_t paramType; // 先使用int32_t
#endif

    /* UI_STATE 描述当前UI运行状态 */
    typedef enum
    {
        STATE_NONE = 0,      // 当前无状态
        STATE_RUN_PAGE_DOWN, // 当前正在运行菜单
        STATE_RUN_PAGE_UP,
        STATE_READY_TO_JUMP_PAGE, // 准备跳转状态 在此状态设置跳转参数
        STATE_JUMP_PAGE,          // 当前进行页面跳转
        STATE_JUMP_PAGE_ARRIVE,   // 当前页面跳转结束
    } HugoUIState_e;

    /* Item的functype类型 */
    typedef enum
    {
        ITEM_PAGE_DESCRIPTION, // 此页的描述
        ITEM_CALL_FUNCTION,    // 此挂件打开就是函数
        ITEM_JUMP_PAGE,        // 跳转的下一页or上一页
        ITEM_SWITCH,           // 开关on&off
        ITEM_CHANGE_VALUE,     // 改变值
        ITEM_CHECKBOX,
        ITEM_MESSAGE,
    } HugoUIItem_e;

    /* Page的functype类型 */
    typedef enum
    {
        PAGE_LIST,   // 该页是列表
        PAGE_CUSTOM, // 该页是定做的(icon)
    } HugoUIPage_e;

    /* Item的结构体 */
    typedef struct HugoUI_item // 小挂件的结构体
    {
        struct HugoUI_item *next;
        HugoUIItem_e funcType; // 作用类型
        uint16_t itemId;        // 小挂件的id
        uint16_t lineId;        // 在每一页的id
        // float item_x, item_x_trg;
        // float item_y, item_y_trg;
        uint8_t *pic;
        // uint8_t step;
        char *title; // 小挂件的名字
        char *msg;   // ITEM_MESSAGE
        char *desc;
        bool *flag; // ITEM_CHECKBOX and ITEM_RADIO_BUTTON and ITEM_SWITCH //小挂件用于这些作用时的标志位
        // bool flagDefault; // Factory default setting//恢复出厂设置
        paramType *param;                                                     // ITEM_CHANGE_VALUE and ITEM_PROGRESS_BAR//小挂件可改变的参数
        uint8_t inPage;                                                       // ITEM_JUMP_PAGE//Item在哪一页
        uint8_t JumpPage;                                                     // 将要跳转到哪一个page
        uint8_t JumpItem;                                                     // 将要跳转到哪一个Item
        void (*FuncCallBack)(void);                                           // 回调函数// ITEM_CHANGE_VALUE and ITEM_PROGRESS_BAR//该挂件的函数
        struct HugoUI_item *(*SetIconSrc)(const uint8_t *pic);                // 传入图片
        struct HugoUI_item *(*SetJumpId)(uint8_t pageId, uint8_t itemLineId); // 传入PageId和ItemLineId
        struct HugoUI_item *(*SetDescripition)(char *desc);                   // 传入descripition
        struct HugoUI_item *(*ReturnThisItem)(struct HugoUI_item *thisItem);  // 传入descripition
    } HugoUIItem_t;

    /* Page的结构体 */
    typedef struct HugoUI_page
    {
        struct HugoUI_page *next;
        HugoUIPage_e funcType;
        HugoUIItem_t *itemHead, *itemTail;
        char *title; // 该页的名字
        uint8_t pageId;
        uint16_t itemMax; // 该page含有的item数
        float page_x, page_x_trg;
        float page_y, page_y_trg;
        float page_y_forlist, page_y_forlist_trg;

        void (*FuncCallBack)(void);
        struct HugoUI_page *(*SetPgaeFunCallBack)(void (*FuncCallBack)(void));

        void (*PageEventProc)(void);
        struct HugoUI_page *(*SetPgaeEventProc)(void (*PageEventProc)(void));

        void (*PageUIShow)(struct HugoUI_page *thispage, HugoUIItem_t *thisitem);
        struct HugoUI_page *(*SetPageUIShow)(void (*PgagUIShow)(struct HugoUI_page *thispage, HugoUIItem_t *thisitem));

        HugoUIItem_t *(*AddItem)(struct HugoUI_page *thisPage, char *title, HugoUIItem_e itemType, ...);
    } HugoUIPage_t;

    /* 执行频率结构体 */
    typedef struct
    {
        unsigned int executeT;
        unsigned int last_timestamp;
    } HugoUIRate_t;

    /* 在user.c中完善枚举enum Slide_space_Obj 该枚举中每一个枚举名按顺序对应Slide_space结构体数组即可*/
    /**
     *	@名称 Slide_Bar
     *	@变量
     *		x    值
     *		min  最小值
     *		max  最大值
     *		step 步进
     */
    struct Slide_Bar
    {
        float *val; // 值
        float min;
        float max;
        float step;
    };

    /* API-Function */
    void HugoUI_System(void);

    void HugoUI_Control(void);

    void HugoUI_Ticks(void);

    uint8_t HugoUIExecuteRate(HugoUIRate_t *er);

    HugoUIItem_t *ReturnThisItem(HugoUIItem_t *thisItem);

    HugoUIItem_t *SetJumpId(uint8_t pageId, uint8_t itemLineId);

    HugoUIItem_t *SetIconSrc(const uint8_t *pic);

    HugoUIItem_t *SetDescripition(char *desc);

    HugoUIItem_t *AddItem(HugoUIPage_t *thisPage, char *title, HugoUIItem_e itemType, ...);

    HugoUIPage_t *AddPage(HugoUIPage_e mode, char *name);

    uint16_t findParam_forChangeVal(float *param);

    uint8_t HugoUI_Animation_Linear(float *a, float *a_trg, uint8_t n);
    uint8_t HugoUI_Animation_EasyIn(float *a, float *a_trg, uint16_t n);
    uint8_t HugoUI_Animation_EasyOut(float *a, float *a_trg, uint16_t n);
    uint8_t HugoUI_Animation_Blur(void);

    /* 声明变量 */
    static unsigned int page_timestamp = 0;

    extern HugoUIRate_t Rate5Hz;
    extern HugoUIRate_t Rate10Hz;
    extern HugoUIRate_t Rate20Hz;
    extern HugoUIRate_t Rate50Hz;
    extern HugoUIRate_t Rate60Hz;
    extern HugoUIRate_t Rate100Hz;
    extern HugoUIRate_t Rate125Hz;
    extern HugoUIRate_t Rate1000Hz;

    extern uint8_t KeyNum;
    extern uint8_t EncoderNum;

    extern struct Slide_Bar Slide_space[];

    extern HugoUIPage_t *currentPage; // 根据ui_index遍历出当前Page
    extern HugoUIPage_t *lastPage;    // JumpPage时拷贝一份上一个Page
    extern HugoUIItem_t *currentItem; // 根据ui_select遍历出当前Item

#ifdef __cplusplus
}
#endif

#endif
