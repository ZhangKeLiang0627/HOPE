#include <ctype.h>
#include <rt_sys.h>
#include <stdint.h>
#include <time.h>
#include "usart.h"

// 是否将fopen与FatFS关联起来
#define FATFS_EN 1

#if FATFS_EN
#include <ff.h>
#include <stdlib.h>
#endif

// #pragma import(__use_no_semihosting) // 禁用半主机模式
#pragma import(__use_no_semihosting_swi) // 即不使用半主机模式，防止程序进入软件中断
// #pragma import(_main_redirection)

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define IS_STD(fh) ((fh) >= 0 && (fh) <= 2)

/*
 * These names are used during library initialization as the
 * file names opened for stdin, stdout, and stderr.
 * As we define _sys_open() to always return the same file handle,
 * these can be left as their default values.
 */
const char __stdin_name[] = ":kkl";
const char __stdout_name[] = "kkl";
const char __stderr_name[] = "kkl";

FILEHANDLE _sys_open(const char *name, int openmode)
{
#if FATFS_EN
    BYTE mode;
    FIL *fp;
    FRESULT fr;
#endif
    if (name == __stdin_name)
        return STDIN;
    else if (name == __stdout_name)
    {
        uart_init(115200); // 初始化串口 (在main函数执行前执行)
        return STDOUT;
    }
    else if (name == __stderr_name)
        return STDERR;
#if FATFS_EN
    if (sizeof(FILEHANDLE) < sizeof(void *))
    {
        USART1_SendBuf("sizeof(FILEHANDLE) should be no less than sizeof(void *)!\n");
        return -1;
    }
    fp = ff_memalloc(sizeof(FIL));
    if (fp == NULL)
        return -1;

    /* http://elm-chan.org/fsw/ff/doc/open.html */
    if (openmode & OPEN_W)
    {
        mode = FA_CREATE_ALWAYS | FA_WRITE;
        if (openmode & OPEN_PLUS)
            mode |= FA_READ;
    }
    else if (openmode & OPEN_A)
    {
        // mode = FA_OPEN_APPEND | FA_WRITE;
        mode = FA_WRITE;
        if (openmode & OPEN_PLUS)
            mode |= FA_READ;
    }
    else
    {
        mode = FA_READ;
        if (openmode & OPEN_PLUS)
            mode |= FA_WRITE;
    }

    fr = f_open(fp, name, mode);
    if (fr == FR_OK)
        return (uintptr_t)fp;

    ff_memfree(fp);
#endif

    return -1;
}

int _sys_close(FILEHANDLE fh)
{
#if FATFS_EN
    FRESULT fr;
#endif

    if (IS_STD(fh))
    {
        if (fh == STDOUT)
            // usart_deinit();
            return 0;
    }

#if FATFS_EN
    fr = f_close((FIL *)fh);
    if (fr == FR_OK)
    {
        ff_memfree((void *)fh);
        return 0;
    }
#endif

    return -1;
}

int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode)
{
#if FATFS_EN
    FRESULT fr;
    UINT bw;
#endif

    if (fh == STDIN)
        return -1;

    if (fh == STDOUT || fh == STDERR)
    {
        USART1_SendBuf((unsigned char *)buf);
        return 0;
    }

#if FATFS_EN
    fr = f_write((FIL *)fh, buf, len, &bw);
    if (fr == FR_OK)
        return len - bw;
#endif

    return -1;
}

int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode)
{
    // char ch;
    int i = 0;
#if FATFS_EN
    FRESULT fr;
    UINT br;
#endif

    if (fh == STDIN)
    {
        while (i < len)
        {
            //     ch = usart_receive();
            //     if (isprint(ch))
            //     {
            //         buf[i++] = ch;
            //         usart_send(ch);
            //     }
            //     else if (ch == '\r')
            //     {
            //         buf[i++] = '\n';
            //         usart_send('\n');
            //         break;
            //     }
            //     else if (i > 0 && ch == '\b')
            //     {
            //         i--;
            //         usart_send_string("\b \b", 3);
            //     }
            i--;
        }

        return len - i;
    }
    else if (fh == STDOUT || fh == STDERR)
        return -1;

#if FATFS_EN
    fr = f_read((FIL *)fh, buf, len, &br);
    if (fr == FR_OK)
        return len - br;
#endif

    return -1;
}

// 检查句柄是否为终端
int _sys_istty(FILEHANDLE fh)
{
    return IS_STD(fh);
}

int _sys_seek(FILEHANDLE fh, long pos)
{
#if FATFS_EN
    FRESULT fr;

    if (!IS_STD(fh))
    {
        fr = f_lseek((FIL *)fh, pos);
        if (fr == FR_OK)
            return 0;
    }
#endif
    return -1;
}

// 刷星句柄关联的缓冲区
int _sys_ensure(FILEHANDLE fh)

{

    return 0;
}

// 返回文件当前长度
long _sys_flen(FILEHANDLE fh)
{
#if FATFS_EN
    if (!IS_STD(fh))
        return f_size((FIL *)fh);
#endif
    return -1;
}

// 在usart.c中定义了，注释防止重复定义
// void _sys_exit(int status)
//{
////while(1);
//}

int _sys_tmpnam(char *name, int fileno, unsigned maxlength)
{

    return 0;
}

// 将一个字符写入控制台
void _ttywrch(int ch)
{
    USART1_SendChar(ch);
}

int remove(const char *filename)
{

    return 0;
}

int rename(const char *oldname, const char *newname)
{

    return 0;
}

// 定义main函数argv的内容
char *_sys_command_string(char *cmd, int len)
{
    // 可以把命令行内容放入大小为len的cmd缓存区然后返回
    // 也可以直接返回一个字符串
    // return "./foo -f bar";
    return NULL;
}

// time_t time(time_t * time)
//{
//	return 0;
// }

clock_t clock(void)
{
    return 0;
}
