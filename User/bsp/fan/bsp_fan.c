/**
  ******************************************************************************
  * @file    bsp_fan.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   风扇应用函数接口
  ******************************************************************************
  * PUMP 使用的引脚:
  *     FB_FAN1: PC6 
  *     FB_FAN2: PC7 
  *     PWM_CTL: PB0 
  *     FAN_CTL: PC11
  *
  * 工作原理:
  *     引脚 FAN_CTL 控制两个风扇的电源开关
  *     引脚 PWM_CTL 输出PWM调节风速
  *     引脚 FB_FAN1/FB_FAN2 为风扇的反馈信号, 输入PWM波检测, 用于判别风扇是否故障
  *     PC6/PC7 -- 捕获PWM方波
  *     PB0     -- 产生PWM方波信号
  */

#include "bsp_fan.h"     


//*****************************************************************************
//*****************************************************************************
 /**
  * @brief  初始化控制风扇电源的IO -- FAN_CTL:PC11
  * @param  无
  * @retval 无
  */
void FAN_GPIO_Config(void)
{   
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /*开启 GPIOC 的外设时钟 -- 所有都是使用 GPIOC 上的引脚*/
    RCC_APB2PeriphClockCmd(macFAN_CTL_GPIO_CLK, ENABLE); 
    
    /*选择要控制的GPIOC引脚*/
    GPIO_InitStructure.GPIO_Pin = macFAN_CTL_GPIO_PIN;
    /*设置引脚模式为通用推挽输出*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    /*设置引脚速率为50MHz */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    /*调用库函数，初始化GPIOC6*/
    GPIO_Init(macFAN_CTL_GPIO_PORT, &GPIO_InitStructure);
    
    /* 关闭所有风扇 */
    GPIO_ResetBits(macFAN_CTL_GPIO_PORT,macFAN_CTL_GPIO_PIN);
}

//*****************************************************************************
//*****************************************************************************

//继电器版本控制风扇关闭 (控制引脚 PB8)
//注意: 这里使用的是与输出PWM波相同的引脚
void CLOSE_FAN_Config(void)
{   
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /*开启 GPIOC 的外设时钟 -- 所有都是使用 GPIOC 上的引脚*/
    RCC_APB2PeriphClockCmd(GENERAL_TIM_CH3_GPIO_CLK, ENABLE); 
    
    /*选择要控制的GPIOB引脚*/
    GPIO_InitStructure.GPIO_Pin = GENERAL_TIM_CH3_PIN;
    /*设置引脚模式为通用推挽输出 -- 注意这里的引脚模式, 不能设为复用模式*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    /*设置引脚速率为50MHz */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    /*调用库函数，初始化GPIOB8*/
    GPIO_Init(GENERAL_TIM_CH3_PORT, &GPIO_InitStructure);
    
    /* 关闭所有风扇: 将 PB8 置位高位 */
    GPIO_SetBits(GENERAL_TIM_CH3_PORT,GENERAL_TIM_CH3_PIN);
}


/**
  * @brief  通用定时器PWM输出用到的GPIO初始化
  * @param  无
  * @retval 无
  */
static void GENERAL_TIM_GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //输出比较通道3 GPIO 初始化 (TIM3 CH3 -- PWM)
    RCC_APB2PeriphClockCmd(GENERAL_TIM_CH3_GPIO_CLK, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GENERAL_TIM_CH3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_AF_OD;//
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GENERAL_TIM_CH3_PORT, &GPIO_InitStructure);
}


/* ----------------   PWM信号 周期和占空比的计算--------------- */
// ARR ：自动重装载寄存器的值
// CLK_cnt：计数器的时钟，等于 Fck_int / (psc+1) = 72M/(psc+1)
// PWM 信号的周期 T = ARR * (1/CLK_cnt) = ARR*(PSC+1) / 72M
// 占空比 P=CCR/(ARR+1)
/**
  * @brief  通用定时器PWM输出初始化
  * @param  无
  * @retval 无
  * @note   
  */
static void GENERAL_TIM_Mode_Config()
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    // 开启定时器时钟,即内部时钟CK_INT=72M
    GENERAL_TIM_APBxClock_FUN(GENERAL_TIM_CLK,ENABLE);

    /*--------------------时基结构体初始化-------------------------*/
    // 配置周期，这里配置为 25K
    
    // 自动重装载寄存器的值，累计TIM_Period+1个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Period=GENERAL_TIM_PERIOD;
    // 驱动CNT计数器的时钟 = Fck_int/(psc+1)
    TIM_TimeBaseStructure.TIM_Prescaler= GENERAL_TIM_PSC;
    // 时钟分频因子 ，配置死区时间时需要用到
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    // 计数器计数模式，设置为向上计数
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    // 重复计数器的值，没用到不用管
    TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
    // 初始化定时器
    TIM_TimeBaseInit(GENERAL_TIM, &TIM_TimeBaseStructure);

    /*--------------------输出比较结构体初始化-------------------*/	
    
    // 配置为PWM模式1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    // 输出使能
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    // 输出通道电平极性配置	
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    // 输出比较通道 3
    //TIM_OCInitStructure.TIM_Pulse = GENERAL_TIM_CCR3;
    //TIM_OCInitStructure.TIM_Pulse = CCR_Num;
    TIM_OCInitStructure.TIM_Pulse = 20;             //占空比初始化为 50%
    TIM_OC3Init(GENERAL_TIM, &TIM_OCInitStructure);
    
    TIM_OC3PreloadConfig(GENERAL_TIM, TIM_OCPreload_Enable);
    //实现改变占空比的方式
    TIM_ARRPreloadConfig(GENERAL_TIM, ENABLE);      //使能定时器的重载寄存器ARR
    
    
    // 使能计数器
    TIM_Cmd(GENERAL_TIM, ENABLE);
}


/**
  * @brief  通用定时器PWM输出用到的GPIO和PWM模式初始化
  * @param  无
  * @retval 无
  */
void GENERAL_TIM_Init()
{
    GENERAL_TIM_GPIO_Config();
    //GPIO_SetBits(GENERAL_TIM_CH3_PORT,GENERAL_TIM_CH3_PIN);
    GENERAL_TIM_Mode_Config();
}

void DISABLE_PWM_TIM()
{
    TIM_Cmd(GENERAL_TIM, DISABLE);
}

//*****************************************************************************
//*****************************************************************************


//捕获PWM方波
//============================================================================
/*
static void ADVANCE_TIM_GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 输出比较通道(PC8) GPIO 初始化
    RCC_APB2PeriphClockCmd(ADVANCE_TIM_CH3_GPIO_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin =  ADVANCE_TIM_CH3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADVANCE_TIM_CH3_PORT, &GPIO_InitStructure);
}


// ----------------   PWM信号 周期和占空比的计算--------------- 
// ARR ：自动重装载寄存器的值
// CLK_cnt：计数器的时钟，等于 Fck_int / (psc+1) = 72M/(psc+1)
// PWM 信号的周期 T = (ARR+1) * (1/CLK_cnt) = (ARR+1)*(PSC+1) / 72M
// 占空比P=CCR/(ARR+1)

static void ADVANCE_TIM_Mode_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    
    // 开启定时器时钟,即内部时钟CK_INT=72M
    ADVANCE_TIM_APBxClock_FUN(ADVANCE_TIM_CLK,ENABLE);

    //--------------------时基结构体初始化-------------------------
    //TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    // 自动重装载寄存器的值，累计TIM_Period+1个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Period=ADVANCE_TIM_PERIOD;
    // 驱动CNT计数器的时钟 = Fck_int/(psc+1)
    TIM_TimeBaseStructure.TIM_Prescaler= ADVANCE_TIM_PSC;
    // 时钟分频因子 ，配置死区时间时需要用到
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    // 计数器计数模式，设置为向上计数
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    // 重复计数器的值，没用到不用管
    TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
    // 初始化定时器
    TIM_TimeBaseInit(ADVANCE_TIM, &TIM_TimeBaseStructure);

    //--------------------输出比较结构体初始化-------------------
    //TIM_OCInitTypeDef  TIM_OCInitStructure;
    // 配置为PWM模式1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    // 输出使能
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    // 设置占空比大小
    TIM_OCInitStructure.TIM_Pulse = ADVANCE_TIM_PULSE;
    // 输出通道电平极性配置
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    //PC8 -- TIM8 CH3
    TIM_OC3Init(ADVANCE_TIM, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(ADVANCE_TIM, TIM_OCPreload_Enable);

    // 使能计数器
    TIM_Cmd(ADVANCE_TIM, ENABLE);
}

void ADVANCE_TIM_Init(void)
{
    ADVANCE_TIM_GPIO_Config();
    ADVANCE_TIM_Mode_Config();
}
*/



 /**
  * @brief  高级定时器 TIMx,x[1,8] 中断优先级配置
  * @param  无
  * @retval 无
  */
static void ADVANCE_TIM_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // 设置中断组 0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    // 设置中断来源
    NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_IRQ;
    // 设置强制优先级
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    // 设置子优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
/**
  * @brief  高级定时器PWM输入用到的GPIO初始化
  * @param  无
  * @retval 无
  */
static void ADVANCE_TIM_GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(ADVANCE_TIM_CH1_GPIO_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin =  ADVANCE_TIM_CH1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
    GPIO_Init(ADVANCE_TIM_CH1_PORT, &GPIO_InitStructure);
}


///*
// * 注意: TIM_TimeBaseInitTypeDef 结构体里面有5个成员, TIM6和TIM7的寄存器里面只有 
// * TIM_Prescaler和TIM_Period, 所有使用TIM6和TIM7的时候只需初始化这两个成员即可,
// * 另外三个寄存器是通用寄存器和高级定时器才有
// *-----------------------------------------------------------------------------
// *typedef struct
// *{ TIM_Prescaler            都有
// *  TIM_CounterMode          TIMx,x[6,7]没有, 其他都有
// *  TIM_Period               都有
// *  TIM_ClockDivision        TIMx,x[6,7]没有, 其他都有
// *  TIM_RepetitionCounter    TIMx,x[1,8,15,16,17]才有
// *}TIM_TimeBaseInitTypeDef; 
// *-----------------------------------------------------------------------------
// */

/* ----------------   PWM信号 周期和占空比的计算--------------- */
// ARR : 自动装载寄存器的值
// CLK_cnt: 计数器的时钟, 等于 Fck_int / (psc+1) = 72M/(psc+1)
// PWM 信号的周期 T = ARR * (1/CLK_cnt) = ARR*(PSC+1) / 72M
// 占空比 P=CCR/(ARR+1)

/**
  * @brief  高级定时器PWM输入用到的GPIO初始化
  * @param  无
  * @retval 无
  */
static void ADVANCE_TIM_Mode_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    
    // 开启定时器时钟, 即内部时钟 CK_INT=72M
    ADVANCE_TIM_APBxClock_FUN(ADVANCE_TIM_CLK,ENABLE);

    /*--------------------时基机构体初始化-------------------------*/
    //TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    // 自动装载寄存器的值, 累积TIM_Period+1个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Period=ADVANCE_TIM_PERIOD;    
    // 驱动CNT计数器的时钟 = Fck_int/(psc+1)
    TIM_TimeBaseStructure.TIM_Prescaler= ADVANCE_TIM_PSC;   
    // 时钟分频因子, 配置死区时间时需要用到
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;   
    // 计数器计数模式，设置为向上计数
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;   
    // 重复计数器的值，没用到不用管
    TIM_TimeBaseStructure.TIM_RepetitionCounter=0;  
    // 初始化定时器
    TIM_TimeBaseInit(ADVANCE_TIM, &TIM_TimeBaseStructure);

    /*--------------------输入捕获结构体初始化-------------------*/	
    // 使用PWM输入模式时, 需要占用两个捕获寄存器, 一个测周期, 另一个测占空比

    //TIM_ICInitTypeDef  TIM_ICInitStructure;
    // 捕获通道IC1配置
    // 选择捕获通道
    TIM_ICInitStructure.TIM_Channel = ADVANCE_TIM_IC1PWM_CHANNEL;
    // 设置捕获的边沿
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    // 设置捕获通道的信号来自于哪个输入通道, 有直连和非直连两种
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    // 1分频, 即捕获信号的每个有效边沿都捕获
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    // 不滤波
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    // 初始化PWM输入模式
    TIM_PWMIConfig(ADVANCE_TIM, &TIM_ICInitStructure);

    // 当工作做PWM输入模式时, 只需要设置触发信号的那一路即可(用于测量周期)
    // 另外一路(用于测量占空比)会由硬件自带设置, 不需要再配置

    // 捕获通道IC2配置	
    //  TIM_ICInitStructure.TIM_Channel = ADVANCE_TIM_IC1PWM_CHANNEL;
    //  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    //  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
    //  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    //  TIM_ICInitStructure.TIM_ICFilter = 0x0;
    //  TIM_PWMIConfig(ADVANCE_TIM, &TIM_ICInitStructure);

    // 选择输入捕获的触发信号
    TIM_SelectInputTrigger(ADVANCE_TIM, TIM_TS_TI1FP1);		

    // 选择从模式: 复位模式
    // PWM输入模式时,从模式必须工作在复位模式,当捕获开始时,计数器CNT会被复位
    TIM_SelectSlaveMode(ADVANCE_TIM, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(ADVANCE_TIM,TIM_MasterSlaveMode_Enable); 

    // 使能捕获中断,这个中断针对的是主捕获通道(测量周期那个)
    TIM_ITConfig(ADVANCE_TIM, TIM_IT_CC1, ENABLE);	
    // 清除中断标志位
    TIM_ClearITPendingBit(ADVANCE_TIM, TIM_IT_CC1);

    // 使能高级控制定时器, 计数器开始计数
    TIM_Cmd(ADVANCE_TIM, ENABLE);
}
/**
  * @brief  高级定时器PWM输入初始化和用到的GPIO初始化
  * @param  无
  * @retval 无
  */
void ADVANCE_TIM_Init(void)
{
    ADVANCE_TIM_GPIO_Config();
    ADVANCE_TIM_NVIC_Config();
    ADVANCE_TIM_Mode_Config();
}

void DISABLE_ADVANCE_TIM()
{
    TIM_Cmd(ADVANCE_TIM, DISABLE);
}






//****************************************************************
//****************************************************************






static void ADVANCE2_TIM_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // 设置中断组 0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    // 设置中断来源
    NVIC_InitStructure.NVIC_IRQChannel = ADVANCE2_TIM_IRQ;
    // 设置强制优先级
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    // 设置子优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
/**
  * @brief  高级定时器PWM输入用到的GPIO初始化
  * @param  无
  * @retval 无
  */
static void ADVANCE2_TIM_GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(ADVANCE2_TIM_CH1_GPIO_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin =  ADVANCE2_TIM_CH1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
    GPIO_Init(ADVANCE2_TIM_CH1_PORT, &GPIO_InitStructure);
}


/**
  * @brief  高级定时器PWM输入用到的GPIO初始化
  * @param  无
  * @retval 无
  */
static void ADVANCE2_TIM_Mode_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    
    // 开启定时器时钟, 即内部时钟 CK_INT=72M
    ADVANCE2_TIM_APBxClock_FUN(ADVANCE2_TIM_CLK,ENABLE);

    /*--------------------时基机构体初始化-------------------------*/
    //TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    // 自动装载寄存器的值, 累积TIM_Period+1个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Period=ADVANCE2_TIM_PERIOD;    
    // 驱动CNT计数器的时钟 = Fck_int/(psc+1)
    TIM_TimeBaseStructure.TIM_Prescaler= ADVANCE2_TIM_PSC;   
    // 时钟分频因子, 配置死区时间时需要用到
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;   
    // 计数器计数模式，设置为向上计数
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;   
    // 重复计数器的值，没用到不用管
    TIM_TimeBaseStructure.TIM_RepetitionCounter=0;  
    // 初始化定时器
    TIM_TimeBaseInit(ADVANCE2_TIM, &TIM_TimeBaseStructure);

    /*--------------------输入捕获结构体初始化-------------------*/	
    // 使用PWM输入模式时, 需要占用两个捕获寄存器, 一个测周期, 另一个测占空比

    //TIM_ICInitTypeDef  TIM_ICInitStructure;
    // 捕获通道IC1配置
    // 选择捕获通道
    TIM_ICInitStructure.TIM_Channel = ADVANCE2_TIM_IC1PWM_CHANNEL;
    // 设置捕获的边沿
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    // 设置捕获通道的信号来自于哪个输入通道, 有直连和非直连两种
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    // 1分频, 即捕获信号的每个有效边沿都捕获
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    // 不滤波
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    // 初始化PWM输入模式
    TIM_PWMIConfig(ADVANCE2_TIM, &TIM_ICInitStructure);

    // 当工作做PWM输入模式时, 只需要设置触发信号的那一路即可(用于测量周期)
    // 另外一路(用于测量占空比)会由硬件自带设置, 不需要再配置

    // 捕获通道IC2配置	
    //  TIM_ICInitStructure.TIM_Channel = ADVANCE2_TIM_IC1PWM_CHANNEL;
    //  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    //  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
    //  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    //  TIM_ICInitStructure.TIM_ICFilter = 0x0;
    //  TIM_PWMIConfig(ADVANCE2_TIM, &TIM_ICInitStructure);

    // 选择输入捕获的触发信号
    TIM_SelectInputTrigger(ADVANCE2_TIM, TIM_TS_TI1FP1);

    // 选择从模式: 复位模式
    // PWM输入模式时,从模式必须工作在复位模式,当捕获开始时,计数器CNT会被复位
    TIM_SelectSlaveMode(ADVANCE2_TIM, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(ADVANCE2_TIM,TIM_MasterSlaveMode_Enable); 

    // 使能捕获中断,这个中断针对的是主捕获通道(测量周期那个)
    TIM_ITConfig(ADVANCE2_TIM, TIM_IT_CC1, ENABLE);	
    // 清除中断标志位
    TIM_ClearITPendingBit(ADVANCE2_TIM, TIM_IT_CC1);

    // 使能高级控制定时器, 计数器开始计数
    TIM_Cmd(ADVANCE2_TIM, ENABLE);
}


void ADVANCE2_TIM_Init(void)
{
    ADVANCE2_TIM_GPIO_Config();
    ADVANCE2_TIM_NVIC_Config();
    ADVANCE2_TIM_Mode_Config();
}

void DISABLE_ADVANCE2_TIM()
{
    TIM_Cmd(ADVANCE2_TIM, DISABLE);
}



/*********************************************END OF FILE**********************/
