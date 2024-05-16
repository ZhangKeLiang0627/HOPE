#include "stm32f4xx.h"
#include "u8g2.h"
#include "delay.h"
#include "oled.h"

/*移植的是 ssd1312 128*64 的库*/
#define SCL_Pin GPIO_Pin_6
#define SDA_Pin GPIO_Pin_7
#define IIC_GPIO_Port GPIOB

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
        __NOP();
        break;
    case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
        for (uint16_t n = 0; n < 320; n++)
        {
            __NOP();
        }
        break;
    case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
        delay_ms(1);
        break;
    case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
        delay_us(5);
        break;                    // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
        if (arg_int == 1)
        {
            GPIO_SetBits(IIC_GPIO_Port, SCL_Pin);
        }
        else if (arg_int == 0)
        {
            GPIO_ResetBits(IIC_GPIO_Port, SCL_Pin);
        }
        break;                   // arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA: // arg_int=0: Output low at I2C data pin
        if (arg_int == 1)
        {
            GPIO_SetBits(IIC_GPIO_Port, SDA_Pin);
        }
        else if (arg_int == 0)
        {
            GPIO_ResetBits(IIC_GPIO_Port, SDA_Pin);
        }
        break; // arg_int=1: Input dir with pullup high for I2C data pin
    case U8X8_MSG_GPIO_MENU_SELECT:
        u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_NEXT:
        u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_PREV:
        u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_HOME:
        u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 1); // default return value
        break;
    }
    return 1;
}

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32]; /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg)
    {

    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;

    case U8X8_MSG_BYTE_INIT:
        /* add your custom code to init i2c subsystem */
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:

        //		HW_I2cWrite(u8x8,buffer,buf_idx);
        // 居然给我蒙对了！
        if (buf_idx <= 0)
            break;

        /* wait for the busy falg to be reset */
        // while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

        /* start transfer */
        I2C_GenerateSTART(I2C1, ENABLE);
        Oled_I2C_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);

        I2C_Send7bitAddress(I2C1, u8x8_GetI2CAddress(u8x8), I2C_Direction_Transmitter);
        Oled_I2C_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);

        for (uint8_t i = 0; i < buf_idx; i++)
        {
            I2C_SendData(I2C1, buffer[i]);

            Oled_I2C_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
        }

        I2C_GenerateSTOP(I2C1, ENABLE);

        break;

    default:
        return 0;
    }
    return 1;
}

// void HW_I2cWrite(uint8_t *buf,uint8_t len)
//{
//	if(len<=0)
//		return ;

//	/* wait for the busy falg to be reset */
//	while(I2C_GetFlagStatus(I2C1,I2C_FLAG_BUSY));

//	/* start transfer */
//	I2C_GenerateSTART(I2C1,ENABLE);
//	while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
//
//	I2C_Send7bitAddress(I2C1,u8x8_GetI2CAddress(u8x8),I2C_Direction_Transmitter);
//
//	while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
//
//	for(uint8_t i=0;i<len;i++)
//	{
//		I2C_SendData(I2C1,buf[i]);
//
//		while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING));
//	}
//	I2C_GenerateSTOP(I2C1,ENABLE);
//}

void u8g2Init(u8g2_t *u8g2)
{
    //	u8g2_Setup_sh1107_i2c_tk078f288_80x128_f(u8g2, U8G2_R1, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);  // 初始化u8g2结构体
    u8g2_Setup_ssd1312_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay); // 初始化u8g2结构体
    u8g2_InitDisplay(u8g2);                                                                       // 根据所选的芯片进行初始化工作，初始化完成后，显示器处于关闭状态
    u8g2_SetPowerSave(u8g2, 0);                                                                   // 打开显示器
    u8g2_ClearBuffer(u8g2);
}
