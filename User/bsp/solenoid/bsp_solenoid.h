#ifndef __SOLENOID_H
#define __SOLENOID_H


#include "stm32f10x.h"


/*
DCF0(电磁阀1): PD2    -- 电磁阀1控制外部水路进入工作仓
DCF1(电磁阀2): PD1    -- 电磁阀2控制工作仓的液排出
*/

/* 定义SOLENOID连接的GPIO端口, 用户只需要修改下面的代码即可改变控制电磁阀的引脚 */
#define macSOLENOID1_GPIO_PORT      GPIOD                       /* GPIO端口 */
#define macSOLENOID1_GPIO_CLK       RCC_APB2Periph_GPIOC        /* GPIO端口时钟 */
#define macSOLENOID1_GPIO_PIN       GPIO_Pin_2                  /* DCF0 引脚号 */


#define macSOLENOID2_GPIO_PORT      GPIOD                       /* GPIO端口 */
#define macSOLENOID2_GPIO_CLK       RCC_APB2Periph_GPIOD        /* GPIO端口时钟 */
#define macSOLENOID2_GPIO_PIN       GPIO_Pin_1                  /* DCF1 引脚号 */


/* 使用标准的固件库控制IO */
#define SOLENOID1_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macSOLENOID1_GPIO_PORT,macSOLENOID1_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macSOLENOID1_GPIO_PORT,macSOLENOID1_GPIO_PIN)

#define SOLENOID2_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macSOLENOID2_GPIO_PORT,macSOLENOID2_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macSOLENOID2_GPIO_PORT,macSOLENOID2_GPIO_PIN)
                    

void SOLENOID_GPIO_Config(void);


#endif /* __SOLENOID_H */
