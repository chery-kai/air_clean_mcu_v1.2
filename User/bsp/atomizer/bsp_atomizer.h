#ifndef __ATOMIZER_H
#define __ATOMIZER_H


#include "stm32f10x.h"


/*
WHQ_CTL0:   PD4     -- 雾化器A(雾化器0)
WHQ_CTL1:   PD3     -- 雾化器B(雾化器1)
*/

/* 定义ATOMIZER连接的GPIO端口, 用户只需要修改下面的代码即可改变控制雾化器的引脚 */
#define macATOMIZER_A_GPIO_PORT         GPIOD                       /* GPIO端口 */
#define macATOMIZER_A_GPIO_CLK          RCC_APB2Periph_GPIOD        /* GPIO端口时钟 */
#define macATOMIZER_A_GPIO_PIN          GPIO_Pin_4                  /* WHQ_CTL0 引脚号 */


#define macATOMIZER_B_GPIO_PORT         GPIOD                       /* GPIO端口 */
#define macATOMIZER_B_GPIO_CLK          RCC_APB2Periph_GPIOD        /* GPIO端口时钟 */
#define macATOMIZER_B_GPIO_PIN          GPIO_Pin_3                  /* WHQ_CTL1 引脚号 */


/* 使用标准的固件库控制IO */
#define ATOMIZER_A_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macATOMIZER_A_GPIO_PORT,macATOMIZER_A_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macATOMIZER_A_GPIO_PORT,macATOMIZER_A_GPIO_PIN)

#define ATOMIZER_B_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macATOMIZER_B_GPIO_PORT,macATOMIZER_B_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macATOMIZER_B_GPIO_PORT,macATOMIZER_B_GPIO_PIN)


void ATOMIZER_GPIO_Config(void);

#endif /* __ATOMIZER_H */
