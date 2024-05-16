#ifndef __SHT30_H__
#define __SHT30_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f4xx.h"
#include "delay.h"

#define SHT30_IIC_ADDR 0x44

    typedef enum
    {
        /* 软件复位命令 */

        SOFT_RESET_CMD = 0x30A2,
        /*
        单次测量模式
        命名格式：Repeatability_CS_CMD
        CS： Clock stretching
        */
        HIGH_ENABLED_CMD = 0x2C06,
        MEDIUM_ENABLED_CMD = 0x2C0D,
        LOW_ENABLED_CMD = 0x2C10,
        HIGH_DISABLED_CMD = 0x2400,
        MEDIUM_DISABLED_CMD = 0x240B,
        LOW_DISABLED_CMD = 0x2416,

        /*
        周期测量模式
        命名格式：Repeatability_MPS_CMD
        MPS：measurement per second
        */
        HIGH_0_5_CMD = 0x2032,
        MEDIUM_0_5_CMD = 0x2024,
        LOW_0_5_CMD = 0x202F,
        HIGH_1_CMD = 0x2130,
        MEDIUM_1_CMD = 0x2126,
        LOW_1_CMD = 0x212D,
        HIGH_2_CMD = 0x2236,
        MEDIUM_2_CMD = 0x2220,
        LOW_2_CMD = 0x222B,
        HIGH_4_CMD = 0x2334,
        MEDIUM_4_CMD = 0x2322,
        LOW_4_CMD = 0x2329,
        HIGH_10_CMD = 0x2737,
        MEDIUM_10_CMD = 0x2721,
        LOW_10_CMD = 0x272A,
        /* 周期测量模式读取数据命令 */
        READOUT_FOR_PERIODIC_MODE = 0xE000,
    } SHT30_CMD;

    uint8_t SHT30_Read_Value(float *get_Temperature, float *get_Humidity);

#ifdef __cplusplus
}
#endif

#endif
