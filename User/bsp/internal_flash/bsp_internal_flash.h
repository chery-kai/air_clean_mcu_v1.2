#ifndef __INTERNAL_FLASH_H
#define __INTERNAL_FLASH_H

#include "stm32f10x.h"


/* STM32大容量产品每页大小2KByte，中、小容量产品每页大小1KByte */
#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)	//2048
#else
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)	//1024
#endif


//air_clean 配置文件保存地址注解:
/***********************************************************************
 * STM32内部Flash每页大小2KB, 总共256页, 这里设定写内部Flash的第101页,
 * 只要查看工程空间分布使用的Flash小于200KB, 则都没有问题
 * Total ROM Size (Code + RO Data + RW Data) < 200KB
 * 0x0800 0000 + 2*1024*100
************************************************************************/
//重复写 Flash 的同一块区域, 是否会导致 Flash 被写坏？
//如果被写坏, 将会是非常严重的问题!!!

//air_clean 配置文件写入的起始地址与结束地址
#define CONF_START_ADDR  ((uint32_t)(0x08000000+2*1024*100))    //0x08032000
#define CONF_END_ADDR    ((uint32_t)(0x08000000+2*1024*101))

//原示例程序定义:
//#define WRITE_START_ADDR  ((uint32_t)0x08008000)
//#define WRITE_END_ADDR    ((uint32_t)0x0800C000)
//int InternalFlash_Test(void);   //测试函数


//读写Flash状态
typedef enum 
{
    FAILED = 0, 
    PASSED = !FAILED
}RWFlashStatus;


//对外函数声明
void ReadFlashNBtye(uint32_t ReadAddress, uint8_t *ReadBuf, int32_t ReadNum);
//int write_cfg_to_flash();     -- appfunc.c 中定义
//int read_cfg_from_flash();    -- appfunc.c 中定义
void tets_flash_func();

#endif /* __INTERNAL_FLASH_H */

