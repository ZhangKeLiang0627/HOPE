#include "usb_bsp.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////

//*******************************************************************************
// 修改信息
// 无
//////////////////////////////////////////////////////////////////////////////////

// USB主机电源控制口
#define USB_HOST_PWRCTRL PAout(15) // PA15

// USB OTG 底层IO初始化
// pdev:USB OTG内核结构体指针
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // 使能GPIOA时钟
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE); // 使能USB OTG时钟
  // GPIOA11,A12设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12; // PA11/12复用功能输出
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;             // 复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           // 推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       // 100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure); // 初始化

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;    // PA15推挽输出
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; // 输出功能
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure); // 初始化

  USB_HOST_PWRCTRL = 1; // 开启USB HOST电源供电

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_OTG_FS); // PA11,AF10(USB)
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_OTG_FS); // PA12,AF10(USB)
}
// USB OTG 中断设置,开启USB FS中断
// pdev:USB OTG内核结构体指针
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;        // 子优先级3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // 使能通道
  NVIC_Init(&NVIC_InitStructure);                              // 配置
}
// USB OTG 中断设置,开启USB FS中断
// pdev:USB OTG内核结构体指针
void USB_OTG_BSP_DisableInterrupt(void)
{
}
// USB OTG 端口供电设置(本例程未用到)
// pdev:USB OTG内核结构体指针
// state:0,断电;1,上电
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{
}
// USB_OTG 端口供电IO配置(本例程未用到)
// pdev:USB OTG内核结构体指针
void USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
}
// USB_OTG us级延时函数
// 本例程采用SYSTEM文件夹的delay.c里面的delay_us函数实现
// 官方例程采用的是定时器2来实现的.
// usec:要延时的us数.
void USB_OTG_BSP_uDelay(const uint32_t usec)
{
  delay_us(usec);
}
// USB_OTG ms级延时函数
// 本例程采用SYSTEM文件夹的delay.c里面的delay_ms函数实现
// 官方例程采用的是定时器2来实现的.
// msec:要延时的ms数.
void USB_OTG_BSP_mDelay(const uint32_t msec)
{
  delay_ms(msec);
}
