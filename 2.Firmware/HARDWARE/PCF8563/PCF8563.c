#include "PCF8563.h"
#include "myiic2.h"

PCF8563_Tag RTC_Time;

/**
 * @brief Dec2BCD(十进制转BCD编码)
 *
 */
static inline uint8_t Dec2BCD(uint8_t DEC)
{
    return ((uint8_t)(DEC / 10) << 4) + (DEC % 10);
}

/**
 * @brief BCD2Dec(BCD转十进制编码)
 *
 */
static inline uint8_t BCD2Dec(uint8_t BCD)
{
    return (uint8_t)(BCD >> 4) * 10 + (BCD & 0x0f);
}

/**
 * @brief  使用IIC总线往PCF8563的寄存器中写一字节数据
 * @param  addr: 寄存器的地址
 * @param  dat: 	待写入的数据
 * @retval None
 */
static void PCF8563_Write_Byte(unsigned char addr, unsigned char dat)
{
    MyIIC_Write_SingleByte(PCF8563_IIC_ADDR, addr, dat);
}

/**
 * @brief  使用IIC总线从PCF8563的寄存器中读一字节数据
 * @param  addr: 寄存器的地址
 * @retval 读出的一字节数据
 */
static uint8_t PCF8563_Read_Byte(unsigned char addr)
{
    return MyIIC_Read_SingleByte(PCF8563_IIC_ADDR, addr);
}

/**
 * @brief  使用IIC总线往PCF8563的寄存器中写多组数据
 * @param  REG_Address: 寄存器的地址
 * @param  BytesNum: 写入数据的字节数
 * @param  buf: 待写入的数据指针
 * @retval None
 */
static void PCF8563_Write_MultiBytes(uint8_t REG_Address, uint8_t BytesNum, uint8_t *buf)
{
    MyIIC_Write_MultiBytes(PCF8563_IIC_ADDR, REG_Address, BytesNum, buf);
}

/**
 * @brief  使用IIC总线从PCF8563的寄存器中读一字节数据
 * @param  REG_Address: 寄存器的地址
 * @param  BytesNum: 写入数据的字节数
 * @param  buf: 待写入的数据指针
 * @retval 返回读出是否成功 0成功/1失败
 */
static uint8_t PCF8563_Read_MultiBytes(uint8_t REG_Address, uint8_t BytesNum, uint8_t *buf)
{
    return MyIIC_Read_MultiBytes(PCF8563_IIC_ADDR, REG_Address, BytesNum, buf);
}

/**
 * @brief  检查与PCF8563的连接是否正常
 * @param  无
 * @retval 0（成功），1（失败）
 */
unsigned char PCF8563CheckConnection(void)
{
    if (PCF8563_Read_Byte(0x0D) != 0xff)
        return 1;
    else
        return 0;
}

// 月修正数据表
static uint8_t const WeekTable[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};

/**
 * @brief PCF8563_GetTime
 * 获取2000-2099年之间的日期对应的星期
 * 功能描述:输入公历日期得到星期(只允许1901-2099年)
 * year,month,day：公历年月日
 * 返回值：星期号(1~7,代表周一 ~ 周日)
 * @param  无
 * @retval 0（成功），1（失败）
 */
static uint8_t PCF8563_GetWeek(uint16_t year, uint8_t month, uint8_t day)
{
    u16 temp2;
    u8 yearH, yearL;
    yearH = year / 100;
    yearL = year % 100;
    // 如果为21世纪,年份数加100
    if (yearH > 19)
        yearL += 100;
    // 所过闰年数只算1900年之后的
    temp2 = yearL + yearL / 4;
    temp2 = temp2 % 7;
    temp2 = temp2 + day + WeekTable[month - 1];
    if (yearL % 4 == 0 && month < 3)
        temp2--;
    temp2 %= 7;
    if (temp2 == 0)
        temp2 = 7;
    return temp2;
}

/**
 * @brief PCF8563_Init
 * @brief 初始化实时时钟芯片PCF8563
 * @param  无
 * @retval 0（成功），1（失败）
 */
uint8_t PCF8563_Init(void)
{
    uint8_t temp;
    PCF8563_Write_Byte(PCF8563_REG_CONTROL1, 0X08); // 启动时钟 // PCF8563_REG_CONTROL1寄存器的第四位置1表启动时钟

    temp = PCF8563_Read_Byte(PCF8563_REG_SECONDS);

    if (temp & 0x80) // PCF8563_REG_SECONDS寄存器的第八位，为 0 表示芯片内储存的时间有效，为 1 表示储存的时间不可靠
    {
        // 时间不可靠，重新设置吧
        PCF8563_SetTime(2023, 10, 31, 21, 33, 0);

        return PCF8563_GetTime();
    }
    else
    {
        // 获取时间
        return PCF8563_GetTime();
    }
}

/**
 * @brief PCF8563_GetTime
 * @brief 从PCF8563中获取实时时间
 * @param  无
 * @retval 0（成功），1（失败）
 */
uint8_t inline PCF8563_GetTime(void)
{
    uint8_t time_data[7];
    uint8_t temp;
    if (PCF8563_Read_MultiBytes(PCF8563_REG_SECONDS, 7, time_data) == 1)
    {
        // i2c读取数据失败
        return 1;
    }

    if (time_data[0] & 0x80)
    {
        // 时间不可靠
        return 1;
    }
    else
    {
        // 时间可靠
        temp = BCD2Dec(time_data[0] & 0x7f);
        if (temp > 59)
            return 1; // 秒钟范围不正确
        RTC_Time.sec = temp;

        temp = BCD2Dec(time_data[1] & 0x7f);
        if (temp > 59)
            return 1; // 分钟范围不正确
        RTC_Time.min = temp;

        temp = BCD2Dec(time_data[2] & 0x3f);
        if (temp > 23)
            return 1; // 时钟范围不正确
        RTC_Time.hour = temp;

        temp = BCD2Dec(time_data[3] & 0x3f);
        if (temp > 31 || temp == 0)
            return 1; // 日期范围不正确
        RTC_Time.date = temp;

        temp = BCD2Dec(time_data[4] & 0x07);
        if (temp > 6)
            return 1; // 星期范围不正确
        RTC_Time.day = temp + 1;

        temp = BCD2Dec(time_data[5] & 0x1f);
        if (temp > 12 || temp == 0)
            return 1; // 月份范围不正确
        RTC_Time.month = temp;

        temp = BCD2Dec(time_data[6]); // 年份
        RTC_Time.year = temp + 2000;
    }

    // 读取时间完成
    return 0;
}

/**
 * @brief PCF8563_SetTime
 * @brief 为PCF8563设置时间
 * @param  //年 月 日 小时 分钟 秒 // (2023.10.31 21:00:00)
 * @retval 无
 */
void inline PCF8563_SetTime(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec)
{
    uint8_t time_data[7];

    // 把时间限制在二十一世纪了，有点感伤
    if (year < 2000)
        year = 2000;
    if (year > 2099)
        year = 2099;

    time_data[0] = Dec2BCD(sec);
    time_data[1] = Dec2BCD(min);
    time_data[2] = Dec2BCD(hour);
    time_data[3] = Dec2BCD(date);
    time_data[4] = PCF8563_GetWeek(year, month, date) - 1;
    time_data[5] = Dec2BCD(month);
    time_data[6] = Dec2BCD(year - 2000);

    PCF8563_Write_MultiBytes(PCF8563_REG_SECONDS, 7, time_data); // 设置时间

    PCF8563_Write_Byte(PCF8563_REG_CONTROL1, 0x08); // 启动时钟
}
