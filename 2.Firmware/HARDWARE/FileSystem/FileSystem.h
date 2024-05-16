#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H
#include "stm32f4xx.h"
#include "Hugo_UI.h"
#include <stdio.h>
void FileSystem_Init(void);

void EventSaveSettingConfig(void);

void EventLoadSettingConfig(void);

uint8_t HugoUIPageFilesAddItems(HugoUIPage_t *thispage, uint8_t *path);
void HugoUIPageFilesCallBack(void);
#endif
