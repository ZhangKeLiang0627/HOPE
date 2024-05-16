#include "FileSystem.h"
#include "sdio_sdcard.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "fattester.h"
#include "malloc.h"

#include "HugoUI_User.h"

const char SystemSavePath[] = "1:/HugoUISAV.txt";
extern struct Slide_Bar Slide_space[];
extern uint8_t ui_Key_num, ui_Encoder_num; // user可以将自己的实现函数的变量直接赋值给这两个Num

/* 挂载flash的文件系统 */
void FileSystem_Init(void)
{
	FRESULT res;
	uint32_t totalmem, freemem;

	if (!SD_Init()) // 初始化SD卡
	{
		printf("sd OK!\r\n");
		res = f_mount(fs[0], "0:", 1); // 挂载SD卡
		printf("sd = %x\r\n", res);
	}
	else
	{
		printf("sd ERR!\r\n");
	}

	W25QXX_Init();

	res = f_mount(fs[1], "1:", 1); // 挂载FLASH
	printf("flash = %x\r\n", res);

	if (res == 0X0D) // FR_NO_FILESYSTEM // FLASH磁盘,FAT文件系统错误,重新格式化FLASH
	{
		// 格式化FLASH
		res = f_mkfs("1:", 1, 4096); // 格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇

		if (res == 0)
		{
			f_setlabel((const TCHAR *)"1:HugoUI");	// 设置Flash磁盘的名字为：HugoUI
			printf("Flash Disk Format Finish\r\n"); // 格式化完成
		}
		else
			printf("Flash Disk Format Error!\r\n"); // 格式化失败
		delay_ms(300);
	}

	while (exf_getfree("1:", &totalmem, &freemem)) // 得到Flash的总容量和剩余容量
	{
		delay_ms(2);
	}
	printf("FATFS OK!\r\n");
	printf("Flash Total Size:%dMB\r\n", totalmem >> 10); // 显示flash总容量 MB
	printf("Flash Free Size:%dMB\r\n", freemem >> 10);  // 显示flash剩余容量 MB
}

/* 用于保存数据的事件函数 */
void EventSaveSettingConfig(void)
{
	static uint8_t SaveFinish_flag = 1; // 初始化该标志为1

	static float move_x = -10, move_x_trg = 10;
	static float move_y = -10, move_y_trg = (SCREEN_HEIGHT / 2 - 15);
	static float movebox_width = 140, movebox_width_trg = 105;
	static float movebox_height = 80, movebox_height_trg = 30;

	// Enter
	if (SaveFinish_flag)
	{
		Oled_u8g2_ShowStr(0, FONT_HEIGHT, "Config Is Saving...");

		f_open(file, "1:/HugoUISAV.txt", FA_OPEN_ALWAYS | FA_WRITE);

		/* ---------- 屏幕亮度变量 ---------- */
		f_write(file, Slide_space[Slide_space_ScreenBrightness].val, sizeof(&Slide_space[Slide_space_ScreenBrightness].val), &bw);

		/* ---------- 动画速度变量 ---------- */
		f_write(file, Slide_space[Slide_space_Fre_x_speed].val, sizeof(&Slide_space[Slide_space_Fre_x_speed].val), &bw);
		f_write(file, Slide_space[Slide_space_Fre_y_speed].val, sizeof(&Slide_space[Slide_space_Fre_y_speed].val), &bw);
		f_write(file, Slide_space[Slide_space_Fre_width_speed].val, sizeof(&Slide_space[Slide_space_Fre_width_speed].val), &bw);
		f_write(file, Slide_space[Slide_space_Slidbar_y_speed].val, sizeof(&Slide_space[Slide_space_Slidbar_y_speed].val), &bw);
		f_write(file, Slide_space[Slide_space_Page_y_forlist_speed].val, sizeof(&Slide_space[Slide_space_Page_y_forlist_speed].val), &bw);
		f_write(file, Slide_space[Slide_space_Page_x_foricon_speed].val, sizeof(&Slide_space[Slide_space_Page_x_foricon_speed].val), &bw);

		/* ---------- 用户设置变量 ---------- */
		f_write(file, Slide_space[Slide_space_WS2812_R].val, sizeof(&Slide_space[Slide_space_WS2812_R].val), &bw);
		f_write(file, Slide_space[Slide_space_WS2812_G].val, sizeof(&Slide_space[Slide_space_WS2812_G].val), &bw);
		f_write(file, Slide_space[Slide_space_WS2812_B].val, sizeof(&Slide_space[Slide_space_WS2812_B].val), &bw);
		f_write(file, Slide_space[Slide_space_val1].val, sizeof(&Slide_space[Slide_space_val1].val), &bw);
		f_write(file, Slide_space[Slide_space_val2].val, sizeof(&Slide_space[Slide_space_val2].val), &bw);
		f_write(file, Slide_space[Slide_space_val3].val, sizeof(&Slide_space[Slide_space_val3].val), &bw);
		SaveFinish_flag = f_write(file, Slide_space[Slide_space_Volume_Ctrl].val, sizeof(&Slide_space[Slide_space_Volume_Ctrl].val), &bw);
		// 如果保存成功会将该标志位置0

		f_close(file);
	}
	else
	{
		HugoUI_Animation_Linear(&move_x, &move_x_trg, 65);
		HugoUI_Animation_Linear(&move_y, &move_y_trg, 65);
		HugoUI_Animation_Linear(&movebox_width, &movebox_width_trg, 65);
		HugoUI_Animation_Linear(&movebox_height, &movebox_height_trg, 65);

		Oled_u8g2_ShowStr((SCREEN_WIDTH - Oled_u8g2_Get_UTF8_ASCII_PixLen("Saved Success!")) / 2,
						  SCREEN_HEIGHT / 2 + 3, "Saved Success!"); // 居中显示

		Oled_u8g2_ShowUTF8((SCREEN_WIDTH - Oled_u8g2_Get_UTF8_ASCII_PixLen("请长按退出~")) / 2,
						   FONT_HEIGHT * 4, "请长按退出~"); // 居中显示

		Oled_u8g2_SetDrawColor(2); // XOR

		Oled_u8g2_DrawRBox(move_x, move_y, movebox_width, movebox_height, 4);

		Oled_u8g2_SetDrawColor(1);

	}

	// Exit
	if (ui_Key_num == 2)
	{
		SaveFinish_flag = 1;
		move_x = -10;
		move_y = -10;
		movebox_width = 140;
		movebox_height = 80;
	}
}

/* 用于上电载入数据的事件函数 */
void EventLoadSettingConfig(void)
{
	FRESULT res;
	res = f_open(file, SystemSavePath, FA_READ);
	if (res == 0)
	{
		/* ---------- 屏幕亮度变量 ---------- */
		f_read(file, Slide_space[Slide_space_ScreenBrightness].val, sizeof(&Slide_space[Slide_space_ScreenBrightness].val), &bw);

		/* ---------- 动画速度变量 ---------- */
		f_read(file, Slide_space[Slide_space_Fre_x_speed].val, sizeof(&Slide_space[Slide_space_Fre_x_speed].val), &bw);
		f_read(file, Slide_space[Slide_space_Fre_y_speed].val, sizeof(&Slide_space[Slide_space_Fre_y_speed].val), &bw);
		f_read(file, Slide_space[Slide_space_Fre_width_speed].val, sizeof(&Slide_space[Slide_space_Fre_width_speed].val), &bw);
		f_read(file, Slide_space[Slide_space_Slidbar_y_speed].val, sizeof(&Slide_space[Slide_space_Slidbar_y_speed].val), &bw);
		f_read(file, Slide_space[Slide_space_Page_y_forlist_speed].val, sizeof(&Slide_space[Slide_space_Page_y_forlist_speed].val), &bw);
		f_read(file, Slide_space[Slide_space_Page_x_foricon_speed].val, sizeof(&Slide_space[Slide_space_Page_x_foricon_speed].val), &bw);

		/* ---------- 用户设置变量 ---------- */
		f_read(file, Slide_space[Slide_space_WS2812_R].val, sizeof(&Slide_space[Slide_space_WS2812_R].val), &bw);
		f_read(file, Slide_space[Slide_space_WS2812_G].val, sizeof(&Slide_space[Slide_space_WS2812_G].val), &bw);
		f_read(file, Slide_space[Slide_space_WS2812_B].val, sizeof(&Slide_space[Slide_space_WS2812_B].val), &bw);
		f_read(file, Slide_space[Slide_space_val1].val, sizeof(&Slide_space[Slide_space_val1].val), &bw);
		f_read(file, Slide_space[Slide_space_val2].val, sizeof(&Slide_space[Slide_space_val2].val), &bw);
		f_read(file, Slide_space[Slide_space_val3].val, sizeof(&Slide_space[Slide_space_val3].val), &bw);
		f_read(file, Slide_space[Slide_space_Volume_Ctrl].val, sizeof(&Slide_space[Slide_space_Volume_Ctrl].val), &bw);
	}
	f_close(file);
}

uint8_t HugoUIPageFilesAddItems(HugoUIPage_t *thispage, uint8_t *path)
{
	FRESULT res;
	char *fn; /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN, fileinfo.lfsize);
#endif

	res = f_opendir(&dir, (const TCHAR *)path); // 打开一个目录
	if (res == FR_OK)
	{
		printf("\r\n");
		while (1)
		{
			HugoUIItem_e thisItemType = ITEM_PAGE_DESCRIPTION;
			char *name = mymalloc(SRAMIN, sizeof(char[16]));

			res = f_readdir(&dir, &fileinfo); // 读取目录下的一个文件
			if (res != FR_OK || fileinfo.fname[0] == 0)
				break; // 错误了/到末尾了,退出
				// if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
#if _USE_LFN
			fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
			fn = fileinfo.fname;
#endif
			// 遍历赋值名字,确定item类型
			for (uint8_t i = 0;; i++)
			{
				if (*(fn + i) != NULL && i < 13)
				{
					*(name + i) = *(fn + i);
					if (*(fn + i + 1) == NULL)
					{
						thisItemType = ITEM_JUMP_PAGE;
						break;
					}
				}
				else if (i < 15)
					*(name + i) = '.';
				else if (i < 16)
					break;
			}

			// to do switch
			switch (thisItemType)
			{
			case ITEM_PAGE_DESCRIPTION:
				thispage->AddItem(thispage, name, thisItemType); // 打印文件名
				break;
			case ITEM_JUMP_PAGE:
				thispage->AddItem(thispage, name, thisItemType)
					->SetJumpId(0, 0); // 打印文件名
				break;
			}
			// thispage->AddItem(thispage, name, thisItemType); // 打印文件名
		}
	}
	if (lastPage == NULL)
		thispage->AddItem(thispage, "Exit", ITEM_JUMP_PAGE)
			->SetJumpId(0, 0);
	else
		thispage->AddItem(thispage, "Exit", ITEM_JUMP_PAGE)
			->SetJumpId(lastPage->pageId, 0);

	myfree(SRAMIN, fileinfo.lfname);
	return res;
}

void HugoUIPageFilesCallBack(void)
{
	if (ui_Key_num == 1)
	{
		printf("HelloFiles!\r\n");
	}

	if (ui_Key_num == 1 && currentItem->funcType == ITEM_JUMP_PAGE)
	{
		printf("PageFilesJump!\r\n");
		// static char *path = "1:/lua-5.3";
		// // // 遍历赋值名字,确定item类型
		// // for (uint8_t i = 0; *(currentItem->title + i) != NULL; i++)
		// // {
		// // 	*(path + i + 3) = *(currentItem->title + i);
		// // }
		// printf(path);
		// HugoUIPage_t *pageFiles = AddPage(PAGE_LIST, "pageFiles")
		// 							  ->SetPgaeFunCallBack(HugoUIPageFilesCallBack);
		// HugoUIPageFilesAddItems(pageFiles, (uint8_t *)path);

		// printf("malloc used:%d\r\n", my_mem_perused(SRAMIN));
	}
}
