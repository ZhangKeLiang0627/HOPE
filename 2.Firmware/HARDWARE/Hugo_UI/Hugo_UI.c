#include "Hugo_UI.h"
#include "Malloc.h"
#include "usart.h"
#include "HugoUI_User.h"
#include "u8g2.h"
/* ---------- 关键变量存放区 ---------- */
/* ---------- best not to change ---------- */
char *HugoUIVersion = "v2.2.3";
/*定义指向Page结构体指针的头尾指针*/
HugoUIPage_t *pageHead = NULL;
HugoUIPage_t *pageTail = NULL;

HugoUIItem_t *ItemHead = NULL;
HugoUIItem_t *ItemTail = NULL;

/* 以下是HugoUI_Control Func中的关键参数变量 */
HugoUIPage_t *currentPage = NULL; // 根据ui_index遍历出当前Page
HugoUIPage_t *lastPage = NULL;    // JumpPage时拷贝一份上一个Page
HugoUIItem_t *currentItem = NULL; // 根据ui_select遍历出当前Item

/* 记录当前Page和Item的关键变量 */
static int16_t ui_index = 0, ui_select = 0, ui_state = STATE_NONE;

static uint8_t jumpPage_flag = 0;
static uint8_t ChangeVal_flag = 0;

uint8_t ui_Key_num, ui_Encoder_num; // user可以将自己的实现函数的变量直接赋值给这两个Num

float frame_y, frame_y_trg;
float frame_x = 128.0f, frame_x_trg = 0.0f;
float frame_width = 36.0f, frame_width_trg = 36.0f;
float slidbar_y, slidbar_y_trg;

float icon_move_x = 0.0f, icon_move_x_trg = 48.0f;
// float page_x_step = 45.0f;
float icon_desc_y = 0.0f, icon_desc_y_trg = 24.0f;
float icon_rectangle_x = 0.0f, icon_rectangle_x_trg = 13.0f;

uint8_t disappear_flag = 0;

uint8_t ItemUIisRunning = false;

/* 初始化速率变量 */
HugoUIRate_t Rate5Hz = {200, 0};
HugoUIRate_t Rate10Hz = {100, 0};
HugoUIRate_t Rate20Hz = {50, 0};
HugoUIRate_t Rate60Hz = {17, 0};
HugoUIRate_t Rate50Hz = {20, 0};
HugoUIRate_t Rate100Hz = {10, 0};
HugoUIRate_t Rate125Hz = {8, 0};
HugoUIRate_t Rate1000Hz = {1, 0};

/* ---------- 动画函数 ---------- */
/**
 * @brief HugoUI_Effect实现滑动效果
 * @param  *a			当前坐标
 * @param	*a_trg		目标坐标
 * @param 	step		运动速度
 * @param	n 	        减速
 * @retval
 */
// 线性
uint8_t HugoUI_Animation_Linear(float *a, float *a_trg, uint8_t n)
{
    if (*a == *a_trg)
        return 0;

    else
    {
        if (fabs(*a - *a_trg) < 0.15f)
            *a = *a_trg;
        else
            *a += (*a_trg - *a) / (n / 10.0f);
    }

    return 1;
}

// 缓慢结束
uint8_t HugoUI_Animation_EasyOut(float *a, float *a_trg, uint16_t n)
{
    if (*a == *a_trg)
        return 0;

    float cz = fabs(*a - *a_trg);

    if (cz <= 1)
        *a = *a_trg;
    else
    {
        if (cz < 10)
            n = n * cz * 0.1f;
        if (n < 10)
            n = 10;
        *a += (*a_trg - *a) / (n * 0.1f);
    }
    return 1;
}
// 缓慢进入
uint8_t HugoUI_Animation_EasyIn(float *a, float *a_trg, uint16_t n)
{
    if (*a == *a_trg)
        return 0;

    float cz = fabs(*a - *a_trg);
    if (cz <= 1)
        *a = *a_trg;
    else if (cz > 20)
        n = n * 3;
    else if (cz > 15)
        n = n * 2;
    else if (cz > 5)
        n = n * 1;
    if (*a != *a_trg)
        *a += (*a_trg - *a) / (n * 0.1f);
    else
        return 0;
    return 1;
}

/**
 * @brief ui模糊转场效果
 * @param  none
 * @retval 1-未完成/0-已完成
 */
uint8_t HugoUI_Animation_Blur(void)
{
    int len = 8 * Oled_u8g2_GetBufferTilHeight() * Oled_u8g2_GetBufferTileWidth();
    uint8_t *p = Oled_u8g2_GetBufferPtr();
    uint8_t return_flag = 0;
    static uint8_t Blur_Effect_temp = 0; // UI模糊转场动效

    if (Blur_Effect_temp >= 6)
    {
        for (int i = 0; i < len; i++)
        {
            if (i % 2 == 0)
                p[i] = p[i] & (0x55);
        }
    }
    if (Blur_Effect_temp >= 13)
    {
        for (int i = 0; i < len; i++)
        {
            if (i % 2 != 0)
                p[i] = p[i] & (0xaa);
        }
    }
    if (Blur_Effect_temp >= 17)
    {
        for (int i = 0; i < len; i++)
        {
            if (i % 2 == 0)
                p[i] = p[i] & (0x00);
        }
    }
    if (Blur_Effect_temp >= 20) // 这段可有可无了
    {
        for (int i = 0; i < len; i++)
        {
            if (i % 2 != 0)
                p[i] = p[i] & (0x00);
        }
    }
    Blur_Effect_temp += 1;
    if (Blur_Effect_temp > 21)
    {
        return_flag = 1;
        Blur_Effect_temp = 0;
    }
    return return_flag ? 0 : 1;
}

/* 用改变值item的*Param去索引Slide_space结构体数组 */
/**
 * @brief findParam_forChangeVal
 * @brief 找到和当前Param对应的Slide_space结构体
 * @param  float *param 当前item的*Param
 * @retval 对应Slide_space结构体在Slide_space结构体数组的位置
 */
uint16_t findParam_forChangeVal(float *param)
{
    uint16_t i = 0;
    for (i = 0; i < Slide_space_Obj_Max; i++)
    {
        if (param == Slide_space[i].val)
        {
            return i;
        }
    }
    return 0;
}

HugoUIItem_t *ReturnThisItem(HugoUIItem_t *thisItem)
{
    thisItem = pageTail->itemTail;

    return thisItem;
}

HugoUIItem_t *SetJumpId(uint8_t pageId, uint8_t itemLineId)
{
    pageTail->itemTail->JumpPage = pageId;
    pageTail->itemTail->JumpItem = itemLineId;

    // printf("thisitemid:%d ", pageTail->itemTail->itemId);
    return pageTail->itemTail;
}

HugoUIItem_t *SetIconSrc(const uint8_t *pic)
{
    pageTail->itemTail->pic = (uint8_t *)pic;
    return pageTail->itemTail;
}

HugoUIItem_t *SetDescripition(char *desc)
{
    pageTail->itemTail->desc = desc;
    return pageTail->itemTail;
}

HugoUIPage_t *SetPageUIShow(void (*PageUIShow)(struct HugoUI_page *thispage, HugoUIItem_t *thisitem))
{
    pageTail->PageUIShow = PageUIShow;
    return pageTail;
}

HugoUIPage_t *SetPgaeFunCallBack(void (*FuncCallBack)(void))
{
    pageTail->FuncCallBack = FuncCallBack;
    return pageTail;
}

HugoUIPage_t *SetPgaeEventProc(void (*PageEventProc)(void))
{
    pageTail->PageEventProc = PageEventProc;
    return pageTail;
}
/**
 * @brief   HugoUI AddItem
 * @param   thisPage         把这个item要放在的page
 * @param   itemType         item的作用类型
 * @param   ...              根据item的作用类型来选填相应的参数
 * @return  void
 */
HugoUIItem_t *AddItem(HugoUIPage_t *thisPage, char *title, HugoUIItem_e itemType, ...)
{
    /*初始化Item结构体，并分配内存*/
    HugoUIItem_t *ItemAdd = mymalloc(SRAMIN, sizeof(HugoUIItem_t));

    ItemAdd->lineId = 0;
    ItemAdd->title = title;             // 给Item命名
    ItemAdd->funcType = itemType;       // 给Item赋予作用类型
    ItemAdd->inPage = thisPage->pageId; // 这个Item在哪一页
    // ItemAdd->item_y = 0;
    // ItemAdd->item_y_trg = 0;

    ItemAdd->SetIconSrc = SetIconSrc; // 初始化item的回调函数
    ItemAdd->SetDescripition = SetDescripition;
    ItemAdd->ReturnThisItem = ReturnThisItem;
    ItemAdd->SetJumpId = NULL;
    ItemAdd->FuncCallBack = NULL;
    ItemAdd->next = NULL;
    /* 让page的尾指针指向这一页 非常重要 Important!!! */
    /* 这个操作是为了在UI初始化函数中的item结构体回调函数连续指->->->做准备 */
    pageTail = thisPage;

    /*给Item头指针赋值*/
    /*生成所有item中的顺序itemid*/
    if (ItemHead == NULL)
    {
        ItemAdd->itemId = 0;
        ItemHead = ItemAdd;
        ItemTail = ItemAdd;
    }
    else // 给开始尾插链表
    {
        ItemAdd->itemId = ItemTail->itemId + 1;
        ItemTail->next = ItemAdd;
        ItemTail = ItemTail->next;
    }

    /*给thisPage的item头指针赋值*/
    /*生成在thisPage中的item顺序lineid*/
    if (thisPage->itemHead == NULL)
    {
        ItemAdd->lineId = 0;
        thisPage->itemHead = ItemAdd;
        thisPage->itemTail = ItemAdd;
    }
    else
    {
        ItemAdd->lineId = thisPage->itemTail->lineId + 1;

        thisPage->itemMax = ItemAdd->lineId + 1;

        thisPage->itemTail->next = ItemAdd;
        thisPage->itemTail = thisPage->itemTail->next;
    }

    printf("thisitemIdInAddItem:%d    ", thisPage->itemTail->itemId);
    printf("thislineIdInAddItem:%d    ", thisPage->itemTail->lineId);

    va_list variableArg;             // 创建一个可变参数列表
    va_start(variableArg, itemType); // 把这个枚举赋值给可变参数列表

    switch (ItemAdd->funcType)
    {
    case ITEM_JUMP_PAGE:
        ItemAdd->SetJumpId = SetJumpId; // 把SetjumpId函数给到这个item
        break;
    case ITEM_CHECKBOX:
    case ITEM_SWITCH:
        ItemAdd->flag = va_arg(variableArg, bool *);
        // ItemAdd->flagDefault = *ItemAdd->flag;
        ItemAdd->FuncCallBack = va_arg(variableArg, void (*)(void));
        break;
    case ITEM_CHANGE_VALUE:
        ItemAdd->param = va_arg(variableArg, paramType *);
        ItemAdd->FuncCallBack = va_arg(variableArg, void (*)(void));
        break;
    case ITEM_MESSAGE:
        ItemAdd->msg = va_arg(variableArg, char *); // massage的内容

    case ITEM_CALL_FUNCTION:
        ItemAdd->FuncCallBack = va_arg(variableArg, void (*)(void));
    default:
        break;
    }
    va_end(variableArg); // 关闭可变参数列表

    return ItemAdd;
}

/**
 * @brief   HugoUI AddPage
 * @param   mode             List/Custom
 * @param   name             PageName
 * @return  HugoUIPage_t*    返回一个Page结构体指针
 */
HugoUIPage_t *AddPage(HugoUIPage_e mode, char *name)
{
    /*初始化结构体,并分配内存*/
    HugoUIPage_t *pageAdd = mymalloc(SRAMIN, sizeof(HugoUIPage_t));

    pageAdd->pageId = 0;
    pageAdd->itemMax = 0;
    pageAdd->page_x = 0;
    pageAdd->page_x_trg = 0;
    pageAdd->page_y = 0;
    pageAdd->page_y_trg = 0;
    pageAdd->page_y_forlist = 0;
    pageAdd->page_y_forlist_trg = 0;

    pageAdd->AddItem = AddItem;
    pageAdd->SetPageUIShow = SetPageUIShow;
    pageAdd->SetPgaeEventProc = SetPgaeEventProc;
    pageAdd->SetPgaeFunCallBack = SetPgaeFunCallBack;
    pageAdd->PageUIShow = NULL;
    pageAdd->PageEventProc = NULL;
    pageAdd->FuncCallBack = NULL;

    pageAdd->itemHead = NULL;
    pageAdd->itemTail = NULL;
    pageAdd->next = NULL;

    /*给此page赋予功能和名字*/
    pageAdd->funcType = mode;
    pageAdd->title = name;

    /*给Page头指针赋值*/ /*生成在所有page中的顺序pageid*/
    if (pageHead == NULL)
    {
        pageAdd->pageId = 0;
        pageHead = pageAdd;
        pageTail = pageAdd;
    }
    else // 给开始尾插链表
    {
        pageAdd->pageId = pageTail->pageId + 1;
        pageTail->next = pageAdd;
        pageTail = pageTail->next;
    }

    printf("thispageidInAddPage:%d    ", pageTail->pageId);

    /*返回Page结构体指针*/
    return pageAdd;
}

/**
 * @brief  提供页面时钟，每1毫秒调用一次
 * @param  无
 * @retval 无
 */
void HugoUI_Ticks(void)
{
    page_timestamp++;
}

/**
 * @brief  在页面循环中通过查询此函数的结果来获得指定的执行频率
 * @param  执行频率相关的结构体指针
 * @retval 无
 */
uint8_t HugoUIExecuteRate(HugoUIRate_t *er)
{
    if (page_timestamp - er->last_timestamp >= er->executeT)
    {
        er->last_timestamp = page_timestamp;
        return 1;
    }
    else
        return 0;
}

// 通用List页面显示
void HugoUICommonListShow(HugoUIPage_t *thispage, HugoUIItem_t *thisitem)
{
    // 计算缓动动画
    if (*Switch_space[SwitchSpace_SmoothAnimation])
    {
        HugoUI_Animation_Linear(&thispage->page_y_forlist, &thispage->page_y_forlist_trg, *Slide_space[Slide_space_Page_y_forlist_speed].val);
        HugoUI_Animation_Linear(&frame_y, &frame_y_trg, *Slide_space[Slide_space_Fre_y_speed].val);
        HugoUI_Animation_Linear(&frame_width, &frame_width_trg, *Slide_space[Slide_space_Fre_width_speed].val);
        HugoUI_Animation_Linear(&slidbar_y, &slidbar_y_trg, *Slide_space[Slide_space_Slidbar_y_speed].val);
    }
    else // 缓动动画标志位为False则没有缓动效果
    {
        thispage->page_y_forlist = thispage->page_y_forlist_trg;
        frame_y = frame_y_trg;
        frame_width = frame_width_trg;
        slidbar_y = slidbar_y_trg;
    }

    // 绘制目录树和目录名
    int16_t Item_x = thispage->page_x;
    int16_t Item_y;
    for (HugoUIItem_t *item = thispage->itemHead; (item->lineId < thispage->itemMax) && (item->inPage == thispage->pageId); item = item->next)
    {
        // 绘制滑动条bar的分隔
        if (item->lineId % 2 == 0)
        {
            Oled_u8g2_DrawLine(thispage->page_x + 125, thispage->page_y + item->lineId * ceil((float)SCREEN_HEIGHT / thispage->itemMax),
                               thispage->page_x + 127, thispage->page_y + item->lineId * ceil((float)SCREEN_HEIGHT / thispage->itemMax));
        }
        else
        {
            Oled_u8g2_DrawLine(thispage->page_x + 125, thispage->page_y + item->lineId * ceil((float)SCREEN_HEIGHT / thispage->itemMax),
                               thispage->page_x + 126, thispage->page_y + item->lineId * ceil((float)SCREEN_HEIGHT / thispage->itemMax));
        }

        // no need to draw at all if we're offscreen
        if (item->lineId <= ui_select - 5 || item->lineId >= ui_select + 6)
            continue;

        Item_y = FONT_HEIGHT - 1 + thispage->page_y + item->lineId * FONT_HEIGHT + thispage->page_y_forlist;
        switch (item->funcType)
        {
        // 此页的描述
        case ITEM_PAGE_DESCRIPTION: // 12是调整距离 // 这底下加的统统都是根据情况微调
            HugoUIDisplayStr(2 + Item_x, Item_y, "-");
            HugoUIDisplayStr(2 + 9 + Item_x, Item_y, item->title);
            break;
        // 页面跳转
        case ITEM_JUMP_PAGE:
            HugoUIDisplayStr(2 + Item_x, Item_y, "+");
            HugoUIDisplayStr(2 + 10 + Item_x, Item_y, item->title);
            break;
        // 勾选框
        case ITEM_CHECKBOX:
            HugoUIDisplayStr(2 + Item_x, Item_y, "-");
            HugoUIDisplayStr(2 + 9 + Item_x, Item_y, item->title);

            // 判断flag画框内的*标记
            if (*item->flag == true)
            {
                HugoUISetDrawColor(2);
                HugoUIDisplayStr(SCREEN_WIDTH - 18 + Item_x, Item_y + 1, "*");
                HugoUISetDrawColor(1);
            }

            // 画单选框
            HugoUIDrawFrame(SCREEN_WIDTH - 20 + Item_x, Item_y - 12 + 3, 11, 11);

            // 当前选项高亮
            if (item->lineId == ui_select)
            {
                HugoUISetDrawColor(2);
                HugoUIDrawBox(SCREEN_WIDTH - 21 + Item_x, Item_y - 12 + 2, 12, 12);
                HugoUISetDrawColor(1);
            }
            break;

        // 开关控件
        case ITEM_SWITCH:
            HugoUIDisplayStr(2 + Item_x, Item_y, "-");
            HugoUIDisplayStr(2 + 9 + Item_x, Item_y, item->title);

            // 摆放 on/off的位置
            HugoUIDisplayStr(SCREEN_WIDTH - FONT_WIDTH * 3 + Item_x, Item_y, *item->flag == true ? "On" : "Off");

            break;

        // 改变值
        case ITEM_CHANGE_VALUE:
            HugoUIDisplayStr(2 + Item_x, Item_y, "-");
            HugoUIDisplayStr(2 + 9 + Item_x, Item_y, item->title);
            // 打印浮点数 判断该浮点数正负来决定宽度
            HugoUIDisplayFloat(*item->param < 100 ? SCREEN_WIDTH - FONT_WIDTH * 4 + Item_x : SCREEN_WIDTH - FONT_WIDTH * 5 + Item_x, Item_y, *item->param, 2, 1);
            // HugoUIDisplayFloat(SCREEN_WIDTH - FONT_WIDTH * 5 , Item_y, *item->param, 2, 1);

            // 当前数字高亮 // ps:如果想要闪烁效果需要获取定时器的时间
            if (item->lineId == ui_select && ChangeVal_flag)
            { // 判断该浮点数正负来决定宽度
                HugoUISetDrawColor(2);
                HugoUIDrawBox(*item->param < 0 ? SCREEN_WIDTH - FONT_WIDTH * 7 + Item_x : SCREEN_WIDTH - FONT_WIDTH * 5 + Item_x, Item_y - 12 + 2, *item->param < 0 ? FONT_WIDTH * 6 : FONT_WIDTH * 4, 11);
                HugoUISetDrawColor(1);
            }
            break;

        default:
            HugoUIDisplayStr(2 + Item_x, Item_y, "-");
            HugoUIDisplayStr(2 + 9 + Item_x, Item_y, item->title);
            break;
        }
    }

    // 绘制frameBox反色选择框
    HugoUISetDrawColor(2);
    HugoUIDrawRBox(thispage->page_x, thispage->page_y + frame_y, thispage->page_x + frame_width + 5, FONT_HEIGHT + 2, 0);
    HugoUISetDrawColor(1);

    // 绘制滑动条slidbar
    Oled_u8g2_DrawLine(thispage->page_x + 126, thispage->page_y, thispage->page_x + 126, thispage->page_y + SCREEN_HEIGHT);
    // 绘制滑动条里的会滚动的box
    HugoUIDrawBox(thispage->page_x + 125, thispage->page_y + slidbar_y, thispage->page_x + 3, ceil((float)SCREEN_HEIGHT / thispage->itemMax));

    // 项目滚动处理
    switch (ui_state)
    {
    case STATE_NONE: // ui无状态

        break;

    case STATE_RUN_PAGE_DOWN: // ui该page向下滚动
                              // 判断该往下滚动多少
                              // 通用滚动顺序

        // textlist的滚动
        if (ui_select >= SCREEN_HEIGHT / 16)
            thispage->page_y_forlist_trg -= FONT_HEIGHT;
        if (ui_select == 0)
            thispage->page_y_forlist_trg = 0;

        // framebox的滚动
        frame_y = frame_y_trg - FONT_HEIGHT * 1.5f;

        if (ui_select < SCREEN_HEIGHT / 16)
        {
            frame_y_trg += FONT_HEIGHT;
        }

        if (ui_select == 0)
            frame_y_trg = 0; // 复位

        frame_width_trg = Oled_u8g2_Get_UTF8_ASCII_PixLen(thisitem->title) + FONT_WIDTH;

        // slidbar的滚动
        slidbar_y_trg = ui_select * ceil((float)SCREEN_HEIGHT / thispage->itemMax);

        ui_state = STATE_NONE;
        break;
    case STATE_RUN_PAGE_UP: // ui 该page向上滚动

        // textlist的滚动
        if (frame_y_trg == 0)
            thispage->page_y_forlist_trg = -ui_select * FONT_HEIGHT;

        // framebox的滚动
        frame_y = frame_y_trg + FONT_HEIGHT * 1.5f;

        frame_y_trg -= FONT_HEIGHT;
        if (frame_y_trg <= 0)
            frame_y_trg = 0;

        frame_width_trg = Oled_u8g2_Get_UTF8_ASCII_PixLen(thisitem->title) + FONT_WIDTH;

        // slidbar的滚动
        slidbar_y_trg = ui_select * ceil((float)SCREEN_HEIGHT / thispage->itemMax);

        ui_state = STATE_NONE;
        break;

    case STATE_READY_TO_JUMP_PAGE: // ui 准备跳转页面
        // 设置page的位置
        thispage->page_x_trg = 0;
        thispage->page_x = 100;

        // 设置frame的位置
        frame_y = SCREEN_HEIGHT * 1.5f;
        frame_y_trg = (ui_select % (SCREEN_HEIGHT / 16)) * FONT_HEIGHT;
        frame_width_trg = Oled_u8g2_Get_UTF8_ASCII_PixLen(thisitem->title) + FONT_WIDTH;

        // 设置slidbar的位置
        slidbar_y = SCREEN_HEIGHT;
        slidbar_y_trg = ui_select * (SCREEN_HEIGHT / (thispage->itemMax + 1));

        // 判断当前list的位置 // 暂时放在这里可以用
        if ((ui_select % (SCREEN_HEIGHT / 16)) - 1)
        {
            if (thispage->page_y_forlist_trg / FONT_HEIGHT == -(ui_select - 1))
                thispage->page_y_forlist_trg -= FONT_HEIGHT;
            else
                thispage->page_y_forlist_trg -= FONT_HEIGHT * (SCREEN_HEIGHT / 16);
        }

        if (ui_select == 0)
            thispage->page_y_forlist_trg = 0;

        ui_state = STATE_JUMP_PAGE;

        break;

    case STATE_JUMP_PAGE: // 目前发现只显示当前页最流畅 // ui 正在跳转页面
        if (HugoUI_Animation_EasyOut(&thispage->page_x, &thispage->page_x_trg, 85) == 0)
        {
            jumpPage_flag |= 0xff;
        }

        if (jumpPage_flag == 0xff)
        {
            jumpPage_flag = 0;
            ui_state = STATE_JUMP_PAGE_ARRIVE;
        }

        break;

    case STATE_JUMP_PAGE_ARRIVE: // ui跳转页面完成
        frame_y_trg = (ui_select % (SCREEN_HEIGHT / 16)) * FONT_HEIGHT;
        frame_width_trg = Oled_u8g2_Get_UTF8_ASCII_PixLen(thisitem->title) + FONT_WIDTH;
        slidbar_y_trg = ui_select * ceil((float)SCREEN_HEIGHT / thispage->itemMax);

        ui_state = STATE_NONE;
        break;

    default:
        break;
    }
}

// 通用Icon页面显示
void HugoUICommonIconShow(HugoUIPage_t *thispage, HugoUIItem_t *thisitem)
{
    // 计算缓动动画
    if (*Switch_space[SwitchSpace_SmoothAnimation]) // 缓动动画标志位 放在了HugoUI_User.c中(in HugoUI_User.c)
    {
        HugoUI_Animation_Linear(&thispage->page_x, &thispage->page_x_trg, *Slide_space[Slide_space_Page_x_foricon_speed].val);
        // HugoUI_Animation_Linear(&frame_y, &frame_y_trg, *Slide_space[Slide_space_Fre_y_speed].val);
        HugoUI_Animation_Linear(&frame_x, &frame_x_trg, *Slide_space[Slide_space_Fre_x_speed].val);
        HugoUI_Animation_Linear(&icon_move_x, &icon_move_x_trg, 75);
        HugoUI_Animation_Linear(&icon_rectangle_x, &icon_rectangle_x_trg, 65);
    }
    else // 缓动动画标志位为False则没有缓动效果
    {
        thispage->page_x = thispage->page_x_trg;
        // frame_y = frame_y_trg;
        frame_x = frame_x_trg;
        icon_move_x = icon_move_x_trg;
        icon_rectangle_x = icon_rectangle_x_trg;
    }
    // 绘制目录树和目录名
    int16_t Item_y = thispage->page_y;
    int16_t Item_x;

    Oled_u8g2_SetBitmapMode(1);
    for (HugoUIItem_t *item = thispage->itemHead; (item->lineId < thispage->itemMax) && (item->inPage == thispage->pageId); item = item->next)
    {
        // no need to draw at all if we're offscreen
        if (item->lineId <= ui_select - 5 || item->lineId >= ui_select + 6)
            continue;

        Item_x = 48 + thispage->page_x + icon_move_x * item->lineId;

        HugoUIDisplayBMP(Item_x, Item_y, 32, 32, item->pic);
    }
    Oled_u8g2_SetBitmapMode(0);

    Oled_u8g2_SetFont(u8g2_font_luBS14_tr);
    HugoUIDisplayStr((128 - Oled_u8g2_Get_UTF8_ASCII_PixLen(thisitem->title)) / 2,
                     Item_y + SCREEN_HEIGHT / 2 + 22 + (FONT_HEIGHT - icon_rectangle_x), thisitem->title); // 居中显示项目名
    Oled_u8g2_SetFont(u8g2_font_wqy13_t_gb2312a);

    HugoUISetDrawColor(2);
    // 绘制frameBox选择框
    HugoUIDrawRBox(48 + frame_x, thispage->page_y, 32, 32, 0);
    // 绘制icon_rectangle
    HugoUIDrawBox(0, thispage->page_y + SCREEN_HEIGHT / 2 + 4, icon_rectangle_x, 24);
    HugoUISetDrawColor(1);

    // 项目滚动处理
    switch (ui_state)
    {
    case STATE_NONE: // ui无状态

        break;

    case STATE_RUN_PAGE_DOWN: // ui该page向下滚动
        // 判断该往下滚动多少
        thispage->page_x_trg = -(ui_select * 48); // 修改ICON的x位移
        frame_x -= 24;                            // frameBox的动效

        icon_rectangle_x = -4; // 复位icon_rectangle

        if (ui_select == 0)
        {
            icon_move_x = 0;
        }

        ui_state = STATE_NONE;
        break;

    case STATE_RUN_PAGE_UP: // ui 该page向上滚动
        // 判断该往上滚动多少
        thispage->page_x_trg = -(ui_select * 48);
        frame_x += 24;

        icon_rectangle_x = -4; // 复位icon_rectangle

        ui_state = STATE_NONE;
        break;

    case STATE_READY_TO_JUMP_PAGE: // ui 准备跳转页面
        // 设置page的位置
        thispage->page_y_trg = 0;
        thispage->page_y = 40;

        icon_move_x = 160; // 形成图标间隙滑动复位的动效
        frame_x = 160;     // 形成选择框滑动归位的动效
        frame_y = SCREEN_HEIGHT * 1.5f;

        thispage->page_x_trg = -(ui_select * 48); // 修改ICON的x位移
        icon_rectangle_x = -4;                    // 复位icon_rectangle

        ui_state = STATE_JUMP_PAGE;
        break;

    case STATE_JUMP_PAGE: // 目前发现只显示当前页最流畅 // ui 正在跳转页面

        if (HugoUI_Animation_EasyIn(&thispage->page_y, &thispage->page_y_trg, 75) == 0)
        {
            jumpPage_flag |= 0xff;
        }

        if (jumpPage_flag == 0xff)
        {
            jumpPage_flag = 0;
            ui_state = STATE_JUMP_PAGE_ARRIVE;
        }

        break;

    case STATE_JUMP_PAGE_ARRIVE: // ui跳转页面完成

        thispage->page_x_trg = -(ui_select * 48); // 修改ICON的x位移

        ui_state = STATE_NONE;
        break;
    default:
        break;
    }
}

void HugoUICommonEventProc(void)
{
    if (ui_state == STATE_NONE)
    {
        /* 设置按键操作 */
        if (ui_Encoder_num == 2)
        {
            if (ChangeVal_flag) // 如果改变值标志位开启
            {
                *currentItem->param += Slide_space[findParam_forChangeVal(currentItem->param)].step;
                // 限制param大小
                if (*currentItem->param >= Slide_space[findParam_forChangeVal(currentItem->param)].max)
                    *currentItem->param = Slide_space[findParam_forChangeVal(currentItem->param)].max;
                // 执行当前Item的cb函数
                if (currentItem->FuncCallBack != NULL)
                    currentItem->FuncCallBack();
            }
            else
            {
                ui_select++;
                ui_state = STATE_RUN_PAGE_DOWN;
                if (ui_select > currentPage->itemTail->lineId)
                    ui_select = 0; // 回到currentPage的item头部
                printf("Enocder Go\r\n");
            }
        }
        else if (ui_Encoder_num == 1)
        {
            if (ChangeVal_flag) // 如果改变值标志位开启
            {
                *currentItem->param -= Slide_space[findParam_forChangeVal(currentItem->param)].step;
                // 限制param大小
                if (*currentItem->param <= Slide_space[findParam_forChangeVal(currentItem->param)].min)
                    *currentItem->param = Slide_space[findParam_forChangeVal(currentItem->param)].min;
                // 执行当前Item的cb函数
                if (currentItem->FuncCallBack != NULL)
                    currentItem->FuncCallBack();
            }
            else
            {
                ui_select--;
                ui_state = STATE_RUN_PAGE_UP;
                if (ui_select < 0)
                    ui_select = currentPage->itemTail->lineId; // 回到currentPage的item尾巴
                printf("Enocder Back\r\n");
            }
        }
        else if (ui_Key_num == 1)
        {
            switch (currentItem->funcType)
            {
            case ITEM_JUMP_PAGE:
                // 页面跳转操作
                ui_state = STATE_READY_TO_JUMP_PAGE; // 设置成准备跳转状态
                ui_index = currentItem->JumpPage;
                ui_select = currentItem->JumpItem;
                // 保留上一页的信息
                lastPage = currentPage;

                // 设置跳转值(在switch(ui_state)的函数中完成)
                break;

            case ITEM_CALL_FUNCTION:
                ItemUIisRunning = true;
                break;

            case ITEM_CHECKBOX:
                // 反转该item的flag
                if (currentItem->flag != NULL)
                    *currentItem->flag = !*currentItem->flag;
                // 执行该item的cb函数
                if (currentItem->FuncCallBack != NULL)
                    currentItem->FuncCallBack();
                break;

            case ITEM_SWITCH:
                // 反转该item的flag
                if (currentItem->flag != NULL)
                    *currentItem->flag = !*currentItem->flag;
                // 执行该item的cb函数
                if (currentItem->FuncCallBack != NULL)
                    currentItem->FuncCallBack();
                break;

            case ITEM_CHANGE_VALUE:
                // 反转ChangeVal_flag
                ChangeVal_flag = !ChangeVal_flag;

                break;

            default:
                break;
            }
            printf("Key single press\r\n");
        }
        else if (ui_Key_num == 2)
        {
            // return Last_page or exit
            // 返回上一页面操作
            if (lastPage != NULL)
            {
                ui_state = STATE_READY_TO_JUMP_PAGE; // 设置成准备跳转状态
                ui_index = lastPage->pageId;
                ui_select = 0;

                // 保留上一页的信息
                lastPage = currentPage;

                ChangeVal_flag = 0; // 把改变值标志位置零 进行强制跳转 同时防止bug
            }
            printf("Key long press\r\n");
        }
    }
}

/* UI主控制函数 */
/**
 * @brief HugoUI_Control
 * @brief 渲染当前Page
 * @param  none
 * @retval none
 */
void HugoUI_Control(void)
{
    if (HugoUIExecuteRate(&Rate60Hz))
    {
        HugoUIClearBuffer(); // u8g2_ScreenClear

        // 若当前Page没有开题图标化则使用普通文本list的模式进行渲染显示 || 开启了PageOnlyList（Page2List）标志位
        if (currentPage->funcType == PAGE_LIST || *Switch_space[PageOnlyListModeConfig])
        {
            HugoUICommonListShow(currentPage, currentItem);
        }
        // 否则使用图形化模式渲染UI
        // else if (currentPage->funcType == PAGE_CUSTOM && currentPage->PgaeUIShow == NULL)
        else if (currentPage->funcType == PAGE_CUSTOM && currentPage->PageUIShow == NULL)
        {
            HugoUICommonIconShow(currentPage, currentItem);
        }
        else if (currentPage->funcType == PAGE_CUSTOM && currentPage->PageUIShow != NULL)
        {
            currentPage->PageUIShow(currentPage, currentItem);
        }

        HugoUISendBuffer(); // u8g2_ScreenRefresh
    }

    /* 执行当前页面的函数 */
    if (currentPage->FuncCallBack != NULL)
        currentPage->FuncCallBack();

    /* Event */
    if (currentPage->PageEventProc != NULL)
        currentPage->PageEventProc();
    else
        HugoUICommonEventProc();
}

void HugoUI_System(void)
{
    // Get ControlNum
    ui_Key_num = KeyNum;
    ui_Encoder_num = EncoderNum;
    KeyNum = 0;
    EncoderNum = 0;

    // Get currentPage by id
    if (currentPage->pageId != ui_index || currentPage == NULL)
    {
        currentPage = pageHead;
        while (currentPage->pageId != ui_index)
        {
            currentPage = currentPage->next;
        }
        printf("pageid:%d\r\n", currentPage->pageId);
    }

    // Get currentItem by id
    if (currentItem->lineId != ui_select || currentItem->inPage != ui_index || currentItem == NULL)
    {
        currentItem = currentPage->itemHead;
        while (currentItem->lineId != ui_select)
        {
            currentItem = currentItem->next;
        }
        printf("itemlid:%d\r\n", currentItem->lineId);
    }

    if (ItemUIisRunning)
    {
        HugoUIClearBuffer();

        if (currentItem->FuncCallBack != NULL)
            currentItem->FuncCallBack();
        else
            ItemUIisRunning = false; // 如果该item没函数就直接退出 防止bug

        if (ui_Key_num == 2)
        {
            ItemUIisRunning = false; // 长按退出应用
        }

        HugoUISendBuffer();
    }
    else
    {
        HugoUI_Control();
    }

    //	printf("runUI\r\n");
}
