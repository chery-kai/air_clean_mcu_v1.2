#ifndef __PUMP_H
#define __PUMP_H


#include "stm32f10x.h"


/*
PUMP_D(液泵1): PC12    -- 水箱液泵, 控制外部加液到水箱
PUMP_X(液泵2): PD0     -- 工作仓液泵, 控制水箱里的液抽到工作仓中
*/

/* 定义PUMP连接的GPIO端口, 用户只需要修改下面的代码即可改变控制液泵的引脚 */
#define macPUMP1_GPIO_PORT      GPIOC                       /* GPIO端口 */
#define macPUMP1_GPIO_CLK       RCC_APB2Periph_GPIOC        /* GPIO端口时钟 */
#define macPUMP1_GPIO_PIN       GPIO_Pin_12                 /* PUMP_D 引脚号 */


#define macPUMP2_GPIO_PORT      GPIOD                       /* GPIO端口 */
#define macPUMP2_GPIO_CLK       RCC_APB2Periph_GPIOD        /* GPIO端口时钟 */
#define macPUMP2_GPIO_PIN       GPIO_Pin_0                  /* PUMP_X 引脚号 */


/* 使用标准的固件库控制IO */
#define PUMP1_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macPUMP1_GPIO_PORT,macPUMP1_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macPUMP1_GPIO_PORT,macPUMP1_GPIO_PIN)

#define PUMP2_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macPUMP2_GPIO_PORT,macPUMP2_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macPUMP2_GPIO_PORT,macPUMP2_GPIO_PIN)
                    

void PUMP_GPIO_Config(void);


#endif /* __PUMP_H */






