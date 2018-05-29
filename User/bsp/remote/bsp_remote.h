#ifndef __REMOTE_H
#define	__REMOTE_H


#include "stm32f10x.h"

/*
//参考: M:\33-TIM—通用定时器\3-TIM—通用定时器-输入捕获-测量脉冲宽度
//************通用定时器TIM参数定义，只限TIM2、3、4、5*************
// 当使用不同的定时器的时候，对应的GPIO是不一样的，这点要注意
// 我们这里默认使用TIM5

#define            GENERAL_TIM                   TIM5
#define            GENERAL_TIM_APBxClock_FUN     RCC_APB1PeriphClockCmd
#define            GENERAL_TIM_CLK               RCC_APB1Periph_TIM5
#define            GENERAL_TIM_PERIOD            0XFFFF
#define            GENERAL_TIM_PSC               (72-1)

// TIM 输入捕获通道GPIO相关宏定义
#define            GENERAL_TIM_CH1_GPIO_CLK      RCC_APB2Periph_GPIOA
#define            GENERAL_TIM_CH1_PORT          GPIOA
#define            GENERAL_TIM_CH1_PIN           GPIO_Pin_0
#define            GENERAL_TIM_CHANNEL_x         TIM_Channel_1

// 中断相关宏定义
#define            GENERAL_TIM_IT_CCx            TIM_IT_CC1
#define            GENERAL_TIM_IRQ               TIM5_IRQn
#define            GENERAL_TIM_INT_FUN           TIM5_IRQHandler

// 获取捕获寄存器值函数宏定义
#define            GENERAL_TIM_GetCapturex_FUN                 TIM_GetCapture1
// 捕获信号极性函数宏定义
#define            GENERAL_TIM_OCxPolarityConfig_FUN           TIM_OC1PolarityConfig

// 测量的起始边沿
#define            GENERAL_TIM_STRAT_ICPolarity                TIM_ICPolarity_Rising
// 测量的结束边沿
#define            GENERAL_TIM_END_ICPolarity                  TIM_ICPolarity_Falling


// 定时器输入捕获用户自定义变量结构体声明
typedef struct
{   
	uint8_t   Capture_FinishFlag;   // 捕获结束标志位
	uint8_t   Capture_StartFlag;    // 捕获开始标志位
	uint16_t  Capture_CcrValue;     // 捕获寄存器的值
	uint16_t  Capture_Period;       // 自动重装载寄存器更新标志 
}TIM_ICUserValueTypeDef;

extern TIM_ICUserValueTypeDef TIM_ICUserValueStructure;

//**************************函数声明********************************
void GENERAL_TIM_Init(void);
*/



/*
//参考: STM32解码红外遥控器的信号.pdf
#define RDATA PBin(9) //红外数据输入脚
//红外遥控识别码(ID),每款遥控器的该值基本都不一样,但也有一样的.
//我们选用的遥控器识别码为 0
#define REMOTE_ID 0
extern u8 RmtCnt; //按键按下的次数
void Remote_Init(void); //红外传感器接收头引脚初始化
u8 Remote_Scan(void);
*/



/*
//
#define PBin9 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)	//PB9
#define RDATA PBin9 //红外数据输入脚
//红外遥控识别码(ID),每款遥控器的该值基本都不一样,但也有一样的.
//我们选用的遥控器识别码为 0
#define REMOTE_ID 0
extern u8 RmtCnt; //按键按下的次数
void Remote_Init(void); //红外传感器接收头引脚初始化
u8 Remote_Scan(void);
*/



//参考: http://blog.csdn.net/qq_16255321/article/details/43602535#comments
//#define RDATA   PAin(1)     //红外数据输入脚  
//#define RDATA GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)
#define RDATA GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)

//红外遥控识别码(ID),每款遥控器的该值基本都不一样,但也有一样的.  
//我们选用的遥控器识别码为0  
#define REMOTE_ID 0xee18                  
extern u8 RmtCnt;           //按键按下的次数  
void Remote_Init(void);     //红外传感器接收头引脚初始化  
u8 Remote_Scan(void);  



#endif /* __REMOTE_H */
