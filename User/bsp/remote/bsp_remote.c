/**
  ******************************************************************************
  * @file    bsp_remote.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   遥控器红外接收应用函数接口
  ******************************************************************************
  */
  
#include "bsp_remote.h"

/*
//参考: M:\33-TIM—通用定时器\3-TIM—通用定时器-输入捕获-测量脉冲宽度
// 定时器输入捕获用户自定义变量结构体定义
TIM_ICUserValueTypeDef TIM_ICUserValueStructure = {0,0,0,0};


// 中断优先级配置
static void GENERAL_TIM_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // 设置中断组为0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    // 设置中断来源
    NVIC_InitStructure.NVIC_IRQChannel = GENERAL_TIM_IRQ ;
    // 设置主优先级为 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    // 设置抢占优先级为3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void GENERAL_TIM_GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 输入捕获通道 GPIO 初始化
    RCC_APB2PeriphClockCmd(GENERAL_TIM_CH1_GPIO_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GENERAL_TIM_CH1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GENERAL_TIM_CH1_PORT, &GPIO_InitStructure);	
}


//
// * 注意：TIM_TimeBaseInitTypeDef结构体里面有5个成员，TIM6和TIM7的寄存器里面只有
// * TIM_Prescaler和TIM_Period，所以使用TIM6和TIM7的时候只需初始化这两个成员即可，
// * 另外三个成员是通用定时器和高级定时器才有.
// *-----------------------------------------------------------------------------
// *typedef struct
// *{ TIM_Prescaler            都有
// *  TIM_CounterMode          TIMx,x[6,7]没有，其他都有
// *  TIM_Period               都有
// *  TIM_ClockDivision        TIMx,x[6,7]没有，其他都有
// *  TIM_RepetitionCounter    TIMx,x[1,8,15,16,17]才有
// *}TIM_TimeBaseInitTypeDef; 
// *-----------------------------------------------------------------------------
// 

// ----------------   PWM信号 周期和占空比的计算--------------- 
// ARR ：自动重装载寄存器的值
// CLK_cnt：计数器的时钟，等于 Fck_int / (psc+1) = 72M/(psc+1)
// PWM 信号的周期 T = ARR * (1/CLK_cnt) = ARR*(PSC+1) / 72M
// 占空比P=CCR/(ARR+1)

static void GENERAL_TIM_Mode_Config(void)
{
    // 开启定时器时钟,即内部时钟CK_INT=72M
    GENERAL_TIM_APBxClock_FUN(GENERAL_TIM_CLK,ENABLE);

    //--------------------时基结构体初始化-------------------------
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
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

    //--------------------输入捕获结构体初始化-------------------
    TIM_ICInitTypeDef TIM_ICInitStructure;
    // 配置输入捕获的通道，需要根据具体的GPIO来配置
    TIM_ICInitStructure.TIM_Channel = GENERAL_TIM_CHANNEL_x;
    // 输入捕获信号的极性配置
    TIM_ICInitStructure.TIM_ICPolarity = GENERAL_TIM_STRAT_ICPolarity;
    // 输入通道和捕获通道的映射关系，有直连和非直连两种
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    // 输入的需要被捕获的信号的分频系数
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    // 输入的需要被捕获的信号的滤波系数
    TIM_ICInitStructure.TIM_ICFilter = 0;
    // 定时器输入捕获初始化
    TIM_ICInit(GENERAL_TIM, &TIM_ICInitStructure);

    // 清除更新和捕获中断标志位
    TIM_ClearFlag(GENERAL_TIM, TIM_FLAG_Update|GENERAL_TIM_IT_CCx);	
    // 开启更新和捕获中断  
    TIM_ITConfig (GENERAL_TIM, TIM_IT_Update | GENERAL_TIM_IT_CCx, ENABLE );

    // 使能计数器
    TIM_Cmd(GENERAL_TIM, ENABLE);
}

void GENERAL_TIM_Init(void)
{
    GENERAL_TIM_GPIO_Config();
    GENERAL_TIM_NVIC_Config();
    GENERAL_TIM_Mode_Config();
}
*/




/*
//参考: STM32解码红外遥控器的信号.pdf
//红外遥控初始化
//设置 IO 以及定时器 4 的输入捕获
void Remote_Init(void)
{
    RCC->APB1ENR|=1<<2; //TIM4 时钟使能
    RCC->APB2ENR|=1<<3; //使能 PORTB 时钟
    GPIOB->CRH&=0XFFFFFF0F; //PB9 输入
    GPIOB->CRH|=0X00000080; //上拉输入
    GPIOB->ODR|=1<<9; //PB9 上拉
    TIM4->ARR=10000; //设定计数器自动重装值 最大 10ms 溢出
    TIM4->PSC=71; //预分频器,1M 的计数频率,1us 加 1.
    TIM4->CCMR2|=1<<8; //CC4S=01 选择输入端 IC4 映射到 TI4 上
    TIM4->CCMR2|=3<<12; //IC4F=0011 配置输入滤波器 8 个定时器时钟周期滤波
    TIM4->CCMR2|=0<<10; //IC4PS=00 配置输入分频,不分频
    TIM4->CCER|=0<<13; //CC4P=0 上升沿捕获
    TIM4->CCER|=1<<12; //CC4E=1 允许捕获计数器的值到捕获寄存器中
    TIM4->DIER|=1<<4; //允许 CC4IE 捕获中断
    TIM4->DIER|=1<<0; //允许更新中断
    TIM4->CR1|=0x01; //使能定时器 4
    //MY_NVIC_Init(1,3,TIM4_IRQChannel,2);//抢占 1，子优先级 3，组 2
    GENERAL_TIM_NVIC_Config();
}
//遥控器接收状态
//[7]:收到了引导码标志
//[6]:得到了一个按键的所有信息
//[5]:保留
//[4]:标记上升沿是否已经被捕获
//[3:0]:溢出计时器
u8 RmtSta=0;
u16 Dval; //下降沿时计数器的值
u32 RmtRec=0; //红外接收到的数据
u8 RmtCnt=0; //按键按下的次数
//定时器 2 中断服务程序
void TIM4_IRQHandler(void)
{
    u16 tsr;
    tsr=TIM4->SR;
    if(tsr&0X01)//溢出
    {
        if(RmtSta&0x80)//上次有数据被接收到了
        {
            RmtSta&=~0X10; //取消上升沿已经被捕获标记
            if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;
            //标记已经完成一次按键的键值信息采集
            if((RmtSta&0X0F)<14)RmtSta++;
            else
            {
                RmtSta&=~(1<<7);//清空引导标识
                RmtSta&=0XF0; //清空计数器
            }
        }
    }
    if(tsr&0x10)//CC4IE 中断
    {
        if(RDATA)//上升沿捕获
        {
            TIM4->CCER|=1<<13; //CC4P=1 设置为下降沿捕获
            TIM4->CNT=0; //清空定时器值
            RmtSta|=0X10; //标记上升沿已经被捕获
        }
        else //下降沿捕获
        {
            Dval=TIM4->CCR4; //读取 CCR1 也可以清 CC1IF 标志位
            TIM4->CCER&=~(1<<13); //CC4P=0 设置为上升沿捕获
            if(RmtSta&0X10) //完成一次高电平捕获
            {
                if(RmtSta&0X80)//接收到了引导码
                {
                    if(Dval>300&&Dval<800) //560 为标准值,560us
                    {
                        RmtRec<<=1; //左移一位.
                        RmtRec|=0; //接收到 0
                    }else if(Dval>1400&&Dval<1800) //1680 为标准值,1680us
                    {
                        RmtRec<<=1; //左移一位.
                        RmtRec|=1; //接收到 1
                    }else if(Dval>2200&&Dval<2600)
                    //得到按键键值增加的信息 2500 为标准值 2.5ms
                    {
                        RmtCnt++; //按键次数增加 1 次
                        RmtSta&=0XF0; //清空计时器
                    }
                }else if(Dval>4200&&Dval<4700)//4500 为标准值 4.5ms
                {
                    RmtSta|=1<<7; //标记成功接收到了引导码
                    RmtCnt=0; //清除按键次数计数器
                }
            }
            RmtSta&=~(1<<4);
        }
    }
    TIM4->SR=0;//清除中断标志位
}
//处理红外键盘
//返回值:
// 0,没有任何按键按下
//其他,按下的按键键值.
u8 Remote_Scan(void)
{
    u8 sta=0;
    u8 t1,t2;
    if(RmtSta&(1<<6))//得到一个按键的所有信息了
    {
        t1=RmtRec>>24; //得到地址码
        t2=(RmtRec>>16)&0xff; //得到地址反码
        if((t1==(u8)~t2)&&t1==REMOTE_ID)//检验遥控识别码(ID)及地址
        {
            t1=RmtRec>>8;
            t2=RmtRec;
            if(t1==(u8)~t2)sta=t1;//键值正确
        }
        
        if((sta==0)||((RmtSta&0X80)==0))//按键数据错误/遥控已经没有按下了
        {
            RmtSta&=~(1<<6);//清除接收到有效按键标识
            RmtCnt=0; //清除按键次数计数器
        }
    }
    return sta;
}
*/


/*
//编译通过 001

// 中断优先级配置
static void GENERAL_TIM_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // 设置中断组为2
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    // 设置中断来源
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn ;        //这里注意
    // 设置主优先级为 1
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    // 设置抢占优先级为3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void Remote_Init(void)
{
    RCC->APB1ENR|=1<<2; //TIM4 时钟使能
    RCC->APB2ENR|=1<<3; //使能 PORTB 时钟
    GPIOB->CRH&=0XFFFFFF0F; //PB9 输入
    GPIOB->CRH|=0X00000080; //上拉输入
    GPIOB->ODR|=1<<9; //PB9 上拉
    TIM4->ARR=10000; //设定计数器自动重装值 最大 10ms 溢出
    TIM4->PSC=71; //预分频器,1M 的计数频率,1us 加 1.
    TIM4->CCMR2|=1<<8; //CC4S=01 选择输入端 IC4 映射到 TI4 上
    TIM4->CCMR2|=3<<12; //IC4F=0011 配置输入滤波器 8 个定时器时钟周期滤波
    TIM4->CCMR2|=0<<10; //IC4PS=00 配置输入分频,不分频
    TIM4->CCER|=0<<13; //CC4P=0 上升沿捕获
    TIM4->CCER|=1<<12; //CC4E=1 允许捕获计数器的值到捕获寄存器中
    TIM4->DIER|=1<<4; //允许 CC4IE 捕获中断
    TIM4->DIER|=1<<0; //允许更新中断
    TIM4->CR1|=0x01; //使能定时器 4
    //MY_NVIC_Init(1,3,TIM4_IRQChannel,2);//抢占 1，子优先级 3，组 2
    GENERAL_TIM_NVIC_Config();
}
//遥控器接收状态
//[7]:收到了引导码标志
//[6]:得到了一个按键的所有信息
//[5]:保留
//[4]:标记上升沿是否已经被捕获
//[3:0]:溢出计时器
u8 RmtSta=0;
u16 Dval; //下降沿时计数器的值
u32 RmtRec=0; //红外接收到的数据
u8 RmtCnt=0; //按键按下的次数


//定时器 2 中断服务程序
void TIM4_IRQHandler(void)
{
    u16 tsr;
    tsr=TIM4->SR;
    if(tsr&0X01)//溢出        
    {
        if(RmtSta&0x80)//上次有数据被接收到了
        {
            RmtSta&=~0X10; //取消上升沿已经被捕获标记
            if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;
            //标记已经完成一次按键的键值信息采集
            if((RmtSta&0X0F)<14)RmtSta++;
            else
            {
                RmtSta&=~(1<<7);//清空引导标识
                RmtSta&=0XF0; //清空计数器
            }
        }
    }
    if(tsr&0x10)//CC4IE 中断
    {
        if(RDATA)//上升沿捕获
        {
            TIM4->CCER|=1<<13; //CC4P=1 设置为下降沿捕获
            TIM4->CNT=0; //清空定时器值
            RmtSta|=0X10; //标记上升沿已经被捕获
        }
        else //下降沿捕获
        {
            Dval=TIM4->CCR4; //读取 CCR1 也可以清 CC1IF 标志位
            TIM4->CCER&=~(1<<13); //CC4P=0 设置为上升沿捕获
            if(RmtSta&0X10) //完成一次高电平捕获
            {
                if(RmtSta&0X80)//接收到了引导码
                {
                    if(Dval>300&&Dval<800) //560 为标准值,560us
                    {
                        RmtRec<<=1; //左移一位.
                        RmtRec|=0; //接收到 0
                    }else if(Dval>1400&&Dval<1800) //1680 为标准值,1680us
                    {
                        RmtRec<<=1; //左移一位.
                        RmtRec|=1; //接收到 1
                    }else if(Dval>2200&&Dval<2600)
                    //得到按键键值增加的信息 2500 为标准值 2.5ms
                    {
                        RmtCnt++; //按键次数增加 1 次
                        RmtSta&=0XF0; //清空计时器
                    }
                }else if(Dval>4200&&Dval<4700)//4500 为标准值 4.5ms
                {
                    RmtSta|=1<<7; //标记成功接收到了引导码
                    RmtCnt=0; //清除按键次数计数器
                }
            }
            RmtSta&=~(1<<4);
        }
    }
    TIM4->SR=0;//清除中断标志位
}
//处理红外键盘
//返回值:
// 0,没有任何按键按下
//其他,按下的按键键值.
u8 Remote_Scan(void)
{
    u8 sta=0;
    u8 t1,t2;
    if(RmtSta&(1<<6))//得到一个按键的所有信息了
    {
        t1=RmtRec>>24; //得到地址码
        t2=(RmtRec>>16)&0xff; //得到地址反码
        if((t1==(u8)~t2)&&t1==REMOTE_ID)//检验遥控识别码(ID)及地址
        {
            t1=RmtRec>>8;
            t2=RmtRec;
            if(t1==(u8)~t2)sta=t1;//键值正确
        }
        
        if((sta==0)||((RmtSta&0X80)==0))//按键数据错误/遥控已经没有按下了
        {
            RmtSta&=~(1<<6);//清除接收到有效按键标识
            RmtCnt=0; //清除按键次数计数器
        }
    }
    return sta;
}

*/







//参考: http://blog.csdn.net/qq_16255321/article/details/43602535#comments

//红外遥控初始化  
//设置IO以及定时器5的输入捕获  
void Remote_Init(void)                  
{
    GPIO_InitTypeDef GPIO_InitStructure;  
    NVIC_InitTypeDef NVIC_InitStructure;  
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;  
    TIM_ICInitTypeDef  TIM_ICInitStructure;    
   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //使能PORTB时钟   
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); //TIM3 时钟使能   
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;           //PA1 输入   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;       //上拉输入   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
    GPIO_SetBits(GPIOA,GPIO_Pin_7); //初始化GPIOA1  
    
    TIM_TimeBaseStructure.TIM_Period = 10000; //设定计数器自动重装值 最大10ms溢出    
    TIM_TimeBaseStructure.TIM_Prescaler =(72-1);    //预分频器,1M的计数频率,1us加1.        
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim  
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式  
  
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx  
    
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;  // 选择输入端 IC2映射到TI5上  
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;   //上升沿捕获  
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;  //配置输入分频,不分频   
    TIM_ICInitStructure.TIM_ICFilter = 0x03;//IC4F=0011 配置输入滤波器 8个定时器时钟周期滤波  
    TIM_ICInit(TIM3, &TIM_ICInitStructure);//初始化定时器输入捕获通道  

    TIM_Cmd(TIM3,ENABLE );    //使能定时器5  
   
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能  
    NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器    
   
    TIM_ITConfig( TIM3,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC2IE捕获中断     
}  
  
   
//遥控器接收状态  
//[7]:收到了引导码标志  
//[6]:得到了一个按键的所有信息  
//[5]:保留      
//[4]:标记上升沿是否已经被捕获                                   
//[3:0]:溢出计时器  
u8  RmtSta=0;           
u16 Dval;       //下降沿时计数器的值  
u32 RmtRec=0;   //红外接收到的数据                
u8  RmtCnt=0;   //按键按下的次数       
//定时器5中断服务程序       
void TIM3_IRQHandler(void)  
{            
//printf("TIM3_IRQHandler \r\n");	
    if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)  
    {  
        if(RmtSta&0x80)//上次有数据被接收到了  
        {     
            RmtSta&=~0X10;                      //取消上升沿已经被捕获标记  
            if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;//标记已经完成一次按键的键值信息采集  
            if((RmtSta&0X0F)<14)RmtSta++;  
            else  
            {  
                RmtSta&=~(1<<7);//清空引导标识  
                RmtSta&=0XF0;   //清空计数器   
            }                                 
        }                                 
    }  
    if(TIM_GetITStatus(TIM3,TIM_IT_CC2)!=RESET)  
    {       
        if(RDATA)//上升沿捕获  
        {  
  
            TIM_OC2PolarityConfig(TIM3,TIM_ICPolarity_Falling);     //CC1P=1 设置为下降沿捕获                 
            TIM_SetCounter(TIM3,0);     //清空定时器值  
            RmtSta|=0X10;                   //标记上升沿已经被捕获  
        }else //下降沿捕获  
        {             
             Dval=TIM_GetCapture2(TIM3);//读取CCR1也可以清CC1IF标志位  
             TIM_OC2PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC4P=0    设置为上升沿捕获  
              
            if(RmtSta&0X10)                 //完成一次高电平捕获   
            {  
                if(RmtSta&0X80)//接收到了引导码  
                {  
                      
                    if(Dval>300&&Dval<800)            //560为标准值,560us  
                    {  
                        RmtRec<<=1;   //左移一位.  
                        RmtRec|=0;  //接收到0       
                    }else if(Dval>1400&&Dval<1800)    //1680为标准值,1680us  
                    {  
                        RmtRec<<=1;   //左移一位.  
                        RmtRec|=1;  //接收到1  
                    }else if(Dval>2200&&Dval<2600)    //得到按键键值增加的信息 2500为标准值2.5ms  
                    {  
                        RmtCnt++;       //按键次数增加1次  
                        RmtSta&=0XF0;   //清空计时器       
                    }  
                }else if(Dval>4200&&Dval<4700)        //4500为标准值4.5ms  
                {  
                    RmtSta|=1<<7; //标记成功接收到了引导码  
                    RmtCnt=0;       //清除按键次数计数器  
                }                          
            }  
            RmtSta&=~(1<<4);  
        }                                                            
    }  
 TIM_ClearFlag(TIM3,TIM_IT_Update|TIM_IT_CC2);        
}  
  
//处理红外键盘  
//返回值:  
//   0,没有任何按键按下  
//其他,按下的按键键值.  

u8 Remote_Scan(void)  
{          
    u8 sta=0; 
    u16 usr_code;
    u8 t1,t2;  
    
    //printf("00001 RmtSta %x\r\n",RmtSta);
    //printf("00001 RmtRec %x\r\n",RmtRec);
    if(RmtSta&(1<<6))//得到一个按键的所有信息了  
    {   
        //printf("\r\n\r\n");
        //printf("[Remote_Scan func] RmtSta ...\r\n");
        usr_code=RmtRec>>16 & 0xffff;                   //得到地址码 
        
        //printf("usr_code = %x \r\n",usr_code);
        if(REMOTE_ID == usr_code)                       //检验遥控识别码(ID)及地址 
        { 
            //printf("[Remote_Scan func] 11111111111111 Right ...\r\n");
            t1=RmtRec>>8;
            t2=RmtRec; 	
            if(t1==(u8)~t2)
                sta=t1;                                 //键值正确
            
            //add 2017.11.20
            RmtSta&=~(1<<6);
            RmtRec = 0x0;
            //printf("[Remote_Scan func] 11111111111111 sta = %X ...\r\n",sta);
        }
        
        if((sta==0)||((RmtSta&0X80)==0))//按键数据错误/遥控已经没有按下了
        {
            //printf("[Remote_Scan func] 22222222222222 Wrong ...\r\n");
            RmtSta&=~(1<<6);                            //清除接收到有效按键标识
            if (RmtCnt!=0)
                //printf("22222222222222 RmtCnt\r\n:%d",RmtCnt);
                RmtCnt=0;                               //清除按键次数计数器
        }
    }

    return sta;  
}  

//咱们的用户码没有使用 (t1==(u8)~t2) 
/*
#define REMOTE_ID01 0xee
u8 Remote_Scan(void)  
{          
    u8 sta=0;
    u8 t1,t2;
    if(RmtSta&(1<<6))//得到一个按键的所有信息了
    {
        t1=RmtRec>>24; //得到地址码
        printf("t1 = %x \r\n",t1);
        t2=(RmtRec>>16)&0xff; //得到地址反码
        printf("t2 = %x \r\n",t2);
        printf("~t2 = %x \r\n",(u8)~t2);
        if((t1==(u8)~t2)&&t1==REMOTE_ID01)//检验遥控识别码(ID)及地址
        {
            t1=RmtRec>>8;
            t2=RmtRec;
            if(t1==(u8)~t2)sta=t1;//键值正确
        }
        
        if((sta==0)||((RmtSta&0X80)==0))//按键数据错误/遥控已经没有按下了
        {
            RmtSta&=~(1<<6);//清除接收到有效按键标识
            RmtCnt=0; //清除按键次数计数器
        }
    }
    
    printf("sta = %x \r\n",sta);
    return sta; 
}  
*/