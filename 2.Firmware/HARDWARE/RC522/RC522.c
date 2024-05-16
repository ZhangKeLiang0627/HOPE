#include "RC522.h"
#include "spi.h"
#include "delay.h"

#define MAXRLEN 18

/**************************************************************************/
/*!
    @brief  init the RST pin

    @param  NULL
*/
/**************************************************************************/
void RC522_RSTPin_Init()
{
    // 初始化结构体
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    // RST-PC14
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);             // 初始化

    GPIO_SetBits(GPIOC, GPIO_Pin_14); // RST设置高
}

/**************************************************************************/
/*!
    @brief  read the Register and return a data

    @param  addr			the address of register that you read

        @returns data 		return the data that you read from the reg
*/
/**************************************************************************/
uint8_t RC522_ReadReg(uint8_t addr)
{
    uint8_t addr_trans, data;
    addr_trans = ((addr << 1) & 0x7E) | 0x80;
    RC522_CS_Clr();

    SPI1_ReadWriteByte(addr_trans);

    data = SPI1_ReadWriteByte(0x00);

    RC522_CS_Set();
    return data;
}

/**************************************************************************/
/*!
    @brief  write the Register

    @param  addr			the address of register that you read
        @param	data 			the data that you write to the reg
        @returns NULL
*/
/**************************************************************************/
void RC522_WriteReg(uint8_t addr, uint8_t dat)
{
    uint8_t addr_trans;
    addr_trans = ((addr << 1) & 0x7E);
    RC522_CS_Clr();
    SPI1_ReadWriteByte(addr_trans);
    SPI1_ReadWriteByte(dat);
    RC522_CS_Set();
}

/**************************************************************************/
/*!
    @brief  Power Off

    @param  NULL

    @returns NULL
*/
/**************************************************************************/
void RC522_PowerOff(void)
{
    RC522_CS_Clr();
    // HAL_Delay(100);
}

/**************************************************************************/
/*!
    @brief  RC522 PORT INIT

    @param  NULL

    @returns NULL
*/
/**************************************************************************/
void RC522_Port_Init(void)
{
    // GPIO initialize
    SPI1_Init();

    RC522_RSTPin_Init();
}

/**************************************************************************/
/*!
    @brief  initialize the RC522

    @param  NULL

        @returns 0 if success; -1 if erro
*/
/**************************************************************************/
uint8_t RC522_Init(void)
{
    uint8_t check_num = 5;
    // GPIO INIT
    RC522_Port_Init();

    // RC522 reset
    RC522_CS_Set();
    delay_ms(100);
    RC522_CS_Clr();
    delay_ms(100);
    RC522_CS_Set();
    delay_ms(100);
    // check
    while ((RC522_ReadReg(0x02) != 0x80) && check_num)
    {
        check_num--;
    }

    RC522_WriteReg(CommandReg, PCD_RESETPHASE);
    RC522_WriteReg(ModeReg, 0x3D); // comunicate with Mifare, CRC initial is 0x6363
    RC522_WriteReg(TReloadRegL, 30);
    RC522_WriteReg(TReloadRegH, 0);
    RC522_WriteReg(TModeReg, 0x8D);
    RC522_WriteReg(TPrescalerReg, 0x3E);
    RC522_WriteReg(TxAutoReg, 0x40);
    // return RC522_ReadReg(0x02);

    if (check_num) // success
    {
        return MI_OK;
    }
    else
    {
        return MI_ERR;
    }
}

void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = RC522_ReadReg(reg);
    RC522_WriteReg(reg, tmp | mask); // set bit mask
}

void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = RC522_ReadReg(reg);
    RC522_WriteReg(reg, tmp & ~mask); // clear bit mask
}

/**************************************************************************/
/*!
    @brief  calculate the CRC16
*/
/**************************************************************************/
void CalculateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;
    ClearBitMask(DivIrqReg, 0x04);
    RC522_WriteReg(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {
        RC522_WriteReg(FIFODataReg, *(pIndata + i));
    }
    RC522_WriteReg(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = RC522_ReadReg(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    pOutData[0] = RC522_ReadReg(CRCResultRegL);
    pOutData[1] = RC522_ReadReg(CRCResultRegM);
}

/**************************************************************************/
/*!
    @brief  comunicate with card

    @param  command
        @param  pInData			the data buffer that trans to the card
        @param  InLenByte		the lenth of pInData
        @param  pOutData		data back from the card
        @param  pOutLenBit	the lenth of pOutData

        @returns the status
*/
/**************************************************************************/
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    RC522_WriteReg(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);
    RC522_WriteReg(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i = 0; i < InLenByte; i++)
    {
        RC522_WriteReg(FIFODataReg, pInData[i]);
    }
    RC522_WriteReg(CommandReg, Command);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    }

    // M1 max wait time is 25ms
    i = 2000;
    do
    {
        n = RC522_ReadReg(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
    {
        if (!(RC522_ReadReg(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = RC522_ReadReg(FIFOLevelReg);
                lastBits = RC522_ReadReg(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++)
                {
                    pOutData[i] = RC522_ReadReg(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    SetBitMask(ControlReg, 0x80); // stop timer now
    RC522_WriteReg(CommandReg, PCD_IDLE);
    return status;
}

/**************************************************************************/
/*!
    @brief  turn on the antenna,the gap bettwen on and off is more than 1ms
*/
/**************************************************************************/
void PcdAntennaOn()
{
    unsigned char i;
    i = RC522_ReadReg(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/**************************************************************************/
/*!
    @brief  turn off the antenna
*/
/**************************************************************************/
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}

/**************************************************************************/
/*!
    @brief  request

    @param  req_code   the way of request
                                                0x52 = find 14443A card
                                                0x26 = find card not sleeped
                                                0x4400 = Mifare_UltraLight
                                                0x0400 = Mifare_One(S50)
                                                0x0200 = Mifare_One(S70)
                                                0x0800 = Mifare_Pro(X)
                                                0x4403 = Mifare_DESFire
        @param  *pTagType	the type of the card

        @returns the status,0 if success
*/
/**************************************************************************/
char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ClearBitMask(Status2Reg, 0x08);
    RC522_WriteReg(BitFramingReg, 0x07);
    SetBitMask(TxControlReg, 0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************************/
/*!
    @brief  wait card off
*/
/**************************************************************************/
void WaitCardOff(void)
{
    char status;
    unsigned char TagType[2];

    while (1)
    {
        status = PcdRequest(REQ_ALL, TagType);
        if (status)
        {
            status = PcdRequest(REQ_ALL, TagType);
            if (status)
            {
                status = PcdRequest(REQ_ALL, TagType);
                if (status)
                {
                    return;
                }
            }
        }
        delay_ms(1000);
    }
}

/**************************************************************************/
/*!
    @brief  PcdAnticoll

    @param  *pSnr  the card number,4 bytes

        @returns the status,0 if success
*/
/**************************************************************************/
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);
    RC522_WriteReg(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80);
    return status;
}

/**************************************************************************/
/*!
    @brief  PcdSelect

    @param  *pSnr  the card number,4 bytes

        @returns the status,0 if success
*/
/**************************************************************************/
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6] ^= *(pSnr + i);
    }
    CalculateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************************/
/*!
    @brief  check the password(key)

    @param  auth_mode   the mode
                                                0x60 = A key
                                                0x61 = B key
        @param  addr		the block address
        @param  *pKey		the key
        @param  *pSnr		the card number,4 bytes

        @returns the status,0 if success
*/
/**************************************************************************/
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 2] = *(pKey + i);
    }
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 8] = *(pSnr + i);
    }

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(RC522_ReadReg(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

char PcdRead(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalculateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

char PcdWrite(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalculateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {

        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalculateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

char PcdHalt(void)
{
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalculateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}

#include "usart.h"

uint8_t status, snr;
uint8_t TagType[4];
uint8_t SelectedSnr[4];
uint8_t DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t read_buf[16];
/* RC522寻卡，并使用串口发送读取的卡号 */
uint8_t RC522_SearchCardForTest(void)
{
    /***********search the card***************/
    uint8_t res = 1;
    uint8_t str2hex_buf[15];

    RC522_Init();

    status = PcdRequest(REQ_ALL, TagType);
    if (!status)
    {
        status = PcdAnticoll(SelectedSnr);
        if (!status)
        {
            status = PcdSelect(SelectedSnr);
            if (!status)
            {
                snr = 0; // sector 0
                status = PcdAuthState(KEYA, (snr * 4 + 3), DefaultKey, SelectedSnr);
                {
                    if (!status)
                    {
                        status = PcdRead((snr * 4 + 0), read_buf);
                        if (!status)
                        {
                            // TODO：记录读取的卡片信息id
                            sprintf(str2hex_buf, "%x %x %x %x\r\n", read_buf[0], read_buf[1], read_buf[2], read_buf[3]);
                            USART6_SendBuf(str2hex_buf);

                            // USART6_SendChar(read_buf[0]);
                            // USART6_SendChar(read_buf[1]);
                            // USART6_SendChar(read_buf[2]);
                            // USART6_SendChar(read_buf[3]);
                            // USART6_SendBuf("\r\n");
                            RC522_PowerOff();
                            res = 0;
                        }
                    }
                }
            }
        }
    }

    /*
    // uint8_t status;
    uint8_t TagType[4];
    uint8_t SelectedSnr[4];

    if (PcdRequest(REQ_ALL, TagType) == MI_OK)
    {
        if (PcdAnticoll(SelectedSnr) == MI_OK)
        {
            USART6_SendBuf(SelectedSnr);
            delay_ms(1000);
        }
    }
    */

    return res;
}

uint8_t RC522_SearchCard(uint8_t *data)
{
    /***********search the card***************/
    uint8_t res = 1;
    uint8_t str2hex_buf[15];

    RC522_Init();

    status = PcdRequest(REQ_ALL, TagType);
    if (!status)
    {
        status = PcdAnticoll(SelectedSnr);
        if (!status)
        {
            status = PcdSelect(SelectedSnr);
            if (!status)
            {
                snr = 0; // sector 0
                status = PcdAuthState(KEYA, (snr * 4 + 3), DefaultKey, SelectedSnr);
                {
                    if (!status)
                    {
                        status = PcdRead((snr * 4 + 0), read_buf);
                        if (!status)
                        {
                            // TODO：记录读取的卡片信息id
                            sprintf(str2hex_buf, "%x %x %x %x\r\n", read_buf[0], read_buf[1], read_buf[2], read_buf[3]);
                            USART6_SendBuf(str2hex_buf);

                            // USART6_SendChar(read_buf[0]);
                            // USART6_SendChar(read_buf[1]);
                            // USART6_SendChar(read_buf[2]);
                            // USART6_SendChar(read_buf[3]);
                            // USART6_SendBuf("\r\n");
                            RC522_PowerOff();

                            // 赋值
                            for (uint8_t i = 0; i < 16; i++)
                            {
                                data[i] = read_buf[i];
                            }

                            res = 0;
                        }
                    }
                }
            }
        }
    }

    /*
    // uint8_t status;
    uint8_t TagType[4];
    uint8_t SelectedSnr[4];

    if (PcdRequest(REQ_ALL, TagType) == MI_OK)
    {
        if (PcdAnticoll(SelectedSnr) == MI_OK)
        {
            USART6_SendBuf(SelectedSnr);
            delay_ms(1000);
        }
    }
    */

    return res;
}

uint8_t RC522_WriteCard(uint8_t *data)
{
    /*************write the cuid************/
    uint8_t res = 1;
    // uint8_t str2hex_buf[15];

    RC522_Init();

    status = PcdRequest(REQ_ALL, TagType);
    if (!status)
    {
        status = PcdAnticoll(SelectedSnr);
        if (!status)
        {
            status = PcdSelect(SelectedSnr);
            if (!status)
            {
                snr = 0; // sector 0
                status = PcdAuthState(KEYA, (snr * 4 + 3), DefaultKey, SelectedSnr);
                {
                    if (!status)
                    {
                        // if (!EEPROM_ERRO)
                        // {
                        //     EEPROM_Read_W_CHECK(data_addr, read_buf, 16);
                        // }

                        status = PcdWrite((snr * 4 + 0), data);
                        if (!status)
                        {
                            // TODO：写卡成功

                            RC522_PowerOff();

                            res = 0;
                        }
                    }
                }
            }
        }
    }

    USART6_SendBuf("写卡结果:");
    USART6_SendBuf(res ? "False" : "true");
    USART6_SendBuf("\r\n");

    return res;
}
