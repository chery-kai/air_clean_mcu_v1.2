/**
  ******************************************************************************
  * @file    bsp_pump.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   pump 应用函数接口
  ******************************************************************************
  * PUMP 使用的引脚:
  *     PUMP_D(液泵1): PC12    -- 水箱液泵, 控制外部加液到水箱
  *     PUMP_X(液泵2): PD0     -- 工作仓液泵, 控制水箱里的液抽到工作仓中
  * 工作原理: 输出高电平打开; 输出低电平关闭
  */
  
#include "bsp_pump.h"     

 /**
  * @brief  初始化控制PUMP的IO
  * @param  无
  * @retval 无
  */
void PUMP_GPIO_Config(void)
{   
    /*定义一个 GPIO_InitTypeDef 类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启 GPIOC 和 GPIOD 的外设时钟*/
    RCC_APB2PeriphClockCmd(macPUMP1_GPIO_CLK|macPUMP2_GPIO_CLK, ENABLE); 

    /*选择要控制的 GPIOC 引脚*/
    GPIO_InitStructure.GPIO_Pin = macPUMP1_GPIO_PIN;

    /*设置引脚模式为通用推挽输出*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

    /*设置引脚速率为50MHz*/   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

    /*调用库函数，初始化 GPIOC12*/
    GPIO_Init(macPUMP1_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    
    /*选择要控制的 GPIOD 引脚*/
    GPIO_InitStructure.GPIO_Pin = macPUMP2_GPIO_PIN;
    
    /*调用库函数，初始化 GPIOD0*/
    GPIO_Init(macPUMP2_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    
    /*关闭所有液泵*/
    GPIO_ResetBits(macPUMP1_GPIO_PORT, macPUMP1_GPIO_PIN);
    GPIO_ResetBits(macPUMP2_GPIO_PORT, macPUMP2_GPIO_PIN);
}
/*********************************************END OF FILE**********************/
