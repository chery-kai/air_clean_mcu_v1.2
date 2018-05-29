#ifndef __FAN_H
#define __FAN_H

#include "stm32f10x.h"

/*
FAN_CTL: PC11

PWM_CTL: PB0

FB_FAN1: PC6 
FB_FAN2: PC7 

PC11    -- 控制风扇电源
PB0     -- 产生PWM方波信号 (通用定时器 TIM3 CH3)
PC6/PC7 -- 捕获PWM方波
*/


//*****************************************************************************
//PC11    -- 控制风扇电源
#define macFAN_CTL_GPIO_PORT        GPIOC                       /* GPIO端口 */
#define macFAN_CTL_GPIO_CLK         RCC_APB2Periph_GPIOC        /* GPIO端口时钟 */
#define macFAN_CTL_GPIO_PIN         GPIO_Pin_11                 /* FAN_CTL 引脚号 */

/* 使用标准的固件库控制IO */
#define FAN_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macFAN_CTL_GPIO_PORT,macFAN_CTL_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macFAN_CTL_GPIO_PORT,macFAN_CTL_GPIO_PIN)



//*****************************************************************************
//PB0     -- 产生PWM方波信号 (通用定时器 TIM3 CH3)
/**
 *  需求分析:
 *      1.风扇的工作频率为 20--30 KHz, 这里取 25KHz
 *      2.需要控制输出PWM波的占空比来调节风速, 这里选取占空比 30%/50%/80%,
 *          分别对应风速: 高/中/低 档
 *  需求简介:  频率 25KHz, 占空比 30%  50%  80%
 */
//PWM 输出示例
//输出PWM的频率为 72M/{ (ARR+1)*(PSC+1) }
//#define            GENERAL_TIM_PERIOD            (10-1)
//#define            GENERAL_TIM_PSC               (72-1)
//#define            GENERAL_TIM_CCR                5
//72M/(10*72)=100KHZ  占空比为 GENERAL_TIM_CCR/(GENERAL_TIM_PERIOD+1) = 50%


//PB0 --> PB8
// TIM3 输出比较通道3
#define            GENERAL_TIM_CH3_GPIO_CLK      RCC_APB2Periph_GPIOB
#define            GENERAL_TIM_CH3_PORT          GPIOB
#define            GENERAL_TIM_CH3_PIN           GPIO_Pin_8//GPIO_Pin_0

#define            GENERAL_TIM                   TIM4//TIM3
#define            GENERAL_TIM_APBxClock_FUN     RCC_APB1PeriphClockCmd
#define            GENERAL_TIM_CLK               RCC_APB1Periph_TIM4//RCC_APB1Periph_TIM3

#define            GENERAL_TIM_PERIOD            (40-1)
#define            GENERAL_TIM_PSC               (72-1) 
//输出PWM的频率: 72M/{ (ARR+1)*(PSC+1)} = 72M/(40*72)=25KHZ, 
//占空比为: GENERAL_TIM_CCR/(GENERAL_TIM_PERIOD+1) = x/40 = 30%,50%,80%
//x 分别取 12, 20, 32




//*****************************************************************************
//*****************************************************************************
//PC6/PC7 -- 捕获PWM方波
#define macFB_FAN1_GPIO_PORT        GPIOC                       /* GPIO端口 */
#define macFB_FAN1_GPIO_CLK         RCC_APB2Periph_GPIOC        /* GPIO端口时钟 */
#define macFB_FAN1_GPIO_PIN         GPIO_Pin_6                  /* FB_FAN1 引脚号 */


#define macFB_FAN2_GPIO_PORT        GPIOC                       /* GPIO端口 */
#define macFB_FAN2_GPIO_CLK         RCC_APB2Periph_GPIOC        /* GPIO端口时钟 */
#define macFB_FAN2_GPIO_PIN         GPIO_Pin_7                  /* FB_FAN2 引脚号 */




//风扇1检测相关
//*****************************************************************************
//*****************************************************************************

#define            ADVANCE_TIM                   TIM8
#define            ADVANCE_TIM_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define            ADVANCE_TIM_CLK               RCC_APB2Periph_TIM8

// 输入捕获能捕获到的最小频率为 72M/{ (ARR+1)*(PSC+1) }
#define            ADVANCE_TIM_PERIOD            (100000-1)
#define            ADVANCE_TIM_PSC               (72-1)

// 中断相关宏定义
#define            ADVANCE_TIM_IRQ               TIM8_CC_IRQn
#define            ADVANCE_TIM_IRQHandler        TIM8_CC_IRQHandler

// TIM1 输入捕获通道1
#define            ADVANCE_TIM_CH1_GPIO_CLK      RCC_APB2Periph_GPIOC
#define            ADVANCE_TIM_CH1_PORT          GPIOC
#define            ADVANCE_TIM_CH1_PIN           GPIO_Pin_6

#define            ADVANCE_TIM_IC1PWM_CHANNEL    TIM_Channel_1
#define            ADVANCE_TIM_IC2PWM_CHANNEL    TIM_Channel_2

void ADVANCE_TIM_Init(void);
void DISABLE_ADVANCE_TIM(void);

//*****************************************************************************
//*****************************************************************************



//风扇2检测相关
//*****************************************************************************
//*****************************************************************************

#define            ADVANCE2_TIM                   TIM1
#define            ADVANCE2_TIM_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define            ADVANCE2_TIM_CLK               RCC_APB2Periph_TIM1

// 输入捕获能捕获到的最小频率为 72M/{ (ARR+1)*(PSC+1) }
#define            ADVANCE2_TIM_PERIOD            (1000-1)
#define            ADVANCE2_TIM_PSC               (72-1)

// 中断相关宏定义
#define            ADVANCE2_TIM_IRQ               TIM1_CC_IRQn
#define            ADVANCE2_TIM_IRQHandler        TIM1_CC_IRQHandler

// TIM1 输入捕获通道1
#define            ADVANCE2_TIM_CH1_GPIO_CLK      RCC_APB2Periph_GPIOA
#define            ADVANCE2_TIM_CH1_PORT          GPIOA
#define            ADVANCE2_TIM_CH1_PIN           GPIO_Pin_8

#define            ADVANCE2_TIM_IC1PWM_CHANNEL    TIM_Channel_1
#define            ADVANCE2_TIM_IC2PWM_CHANNEL    TIM_Channel_2

void ADVANCE2_TIM_Init(void);
void DISABLE_ADVANCE2_TIM(void);

//*****************************************************************************
//*****************************************************************************





/*
//************高级定时器TIM参数定义, 只限TIM1和TIM8************
// 当使用不同的定时器的时候, 对应的GPIO是不一样的, 这点要注意
// 这里我们使用高级定时器 TIM1

#define            ADVANCE_TIM                   TIM1
#define            ADVANCE_TIM_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define            ADVANCE_TIM_CLK               RCC_APB2Periph_TIM1

// 输入捕获能捕获到的最小频率为 72M/{ (ARR+1)*(PSC+1) }
#define            ADVANCE_TIM_PERIOD            (1000-1)
#define            ADVANCE_TIM_PSC               (72-1)

// 中断相关宏定义
#define            ADVANCE_TIM_IRQ               TIM1_CC_IRQn
#define            ADVANCE_TIM_IRQHandler        TIM1_CC_IRQHandler

// TIM1 输入捕获通道1
#define            ADVANCE_TIM_CH1_GPIO_CLK      RCC_APB2Periph_GPIOA
#define            ADVANCE_TIM_CH1_PORT          GPIOA
#define            ADVANCE_TIM_CH1_PIN           GPIO_Pin_8

#define            ADVANCE_TIM_IC1PWM_CHANNEL    TIM_Channel_1
#define            ADVANCE_TIM_IC2PWM_CHANNEL    TIM_Channel_2

//**************************函数声明********************************

void ADVANCE_TIM_Init(void);
*/





//*****************************************************************************
//*****************************************************************************





//*****************************************************************************
void FAN_GPIO_Config(void);
void CLOSE_FAN_Config(void);        //继电器版本控制风扇关闭
void GENERAL_TIM_Init();
void DISABLE_PWM_TIM();
//void ADVANCE_TIM_Init(void);

#endif /* __FAN_H */
