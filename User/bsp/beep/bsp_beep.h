#ifndef __BEEP_H
#define	__BEEP_H


#include "stm32f10x.h"


//引脚修改为 PD8
/******************** BEEP 引脚配置参数定义 **************************/
#define             macBEEP_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define             macBEEP_GPIO_CLK                        RCC_APB2Periph_GPIOD
#define             macBEEP_PORT                            GPIOD
#define             macBEEP_PIN                             GPIO_Pin_8


/******************** BEEP 函数宏定义 **************************/
#define             macBEEP_ON()                            GPIO_SetBits ( macBEEP_PORT, macBEEP_PIN )
#define             macBEEP_OFF()                           GPIO_ResetBits ( macBEEP_PORT, macBEEP_PIN )


/************************** BEEP 函数声明********************************/
void                Beep_Init                               ( void );


#endif /* __BEEP_H */
