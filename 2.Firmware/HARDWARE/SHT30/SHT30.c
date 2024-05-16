#include "SHT30.h"
#include "myiic2.h"
#include "stdio.h"
/**
 * @brief  使用IIC总线往SHT30的设备地址中写16字节数据
 * @param  cmd: 待写入的命令
 * @retval 0为成功 / 1为失败
 */
static uint8_t SHT30_iic_Send_Cmd(SHT30_CMD cmd)
{
    uint8_t i;
    uint8_t cmd_buf[2];
    cmd_buf[0] = cmd >> 8;
    cmd_buf[1] = cmd;
    
    MyIIC_Start();
    MyIIC_SendByte((SHT30_IIC_ADDR << 1) | 0); // 发送器件地址+写命令
    if (MyIIC_ReceiveAck())                // 等待应答
    {
        MyIIC_Stop();
        return 1;
    }
    
    for (i = 0; i < 2; i++)
    {
        MyIIC_SendByte(cmd_buf[i]); // 发送数据
        if (MyIIC_ReceiveAck()) // 等待ACK
        {
            MyIIC_Stop();
            return 1;
        }
    }
    MyIIC_Stop();
    return 0;
}

/**
 * @brief  使用IIC总线从SHT30中读六字节数据
 * @param  buf: 待写入的数据指针
 * @retval 返回读出是否成功 0成功/1失败
 */
static uint8_t SHT30_iic_Read_Datas(uint8_t *buf)
{
    uint8_t DatasNum = 6;
    MyIIC_Start();
    MyIIC_SendByte((SHT30_IIC_ADDR << 1) | 1); // 发送器件地址+读命令
    MyIIC_ReceiveAck();                    // 等待应答
    while (DatasNum)
    {
        if (DatasNum == 1)
            *buf = MyIIC_ReceiveByte(0); // 读数据,发送nACK
        else
            *buf = MyIIC_ReceiveByte(1); // 读数据,发送ACK
        DatasNum--;
        buf++;
    }
    MyIIC_Stop(); // 产生一个停止条件
    return 0;
}

// 温湿度获取函数
// 0成功/1失败
uint8_t SHT30_Read_Value(float *get_Temperature, float *get_Humidity)
{
    u16 tem, hum;
    uint8_t buff[6];
    float Temperature = 0.0f, Humidity = 0.0f;

    if(SHT30_iic_Send_Cmd(HIGH_ENABLED_CMD) == 0)
    {
        SHT30_iic_Read_Datas(buff);
        tem = (uint16_t)((buff[0] << 8) | buff[1]); // 两个温度数据拼接成十六位
        hum = (uint16_t)((buff[3] << 8) | buff[4]); // 两个湿度数据拼接成十六位

        /*转换成实际温湿度*/
        Temperature = (175.0f * (float)tem / 65535.0f - 45.0f); // T = -45 + 175 * tem / (2^16-1)
        Humidity = (100.0f * (float)hum / 65535.0f);           // RH = hum*100 / (2^16-1)

        if ((Temperature >= -20) && (Temperature <= 125) && (Humidity >= 0) && (Humidity <= 100))
        {
            printf("Temperature:%.1f Humidity:%.1f\r\n", Temperature, Humidity);
            *get_Temperature = Temperature;
            *get_Humidity = Humidity;
            return 0;
        }

        return 1;
    }
    return 1;
}
