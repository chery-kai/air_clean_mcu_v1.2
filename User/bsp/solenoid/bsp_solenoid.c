/**
  ******************************************************************************
  * @file    bsp_solenoid.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   solenoid 应用函数接口
  ******************************************************************************
  * SOLENOID 使用的引脚:
  *     DCF0(电磁阀1): PD2    -- 电磁阀1控制外部水路进入工作仓
  *     DCF1(电磁阀2): PD1    -- 电磁阀2控制工作仓的液排出
  * 工作原理: 输出高电平打开; 输出低电平关闭
  */
  
#include "bsp_solenoid.h"     

 /**
  * @brief  初始化控制SOLENOID的IO
  * @param  无
  * @retval 无
  */
void SOLENOID_GPIO_Config(void)
{   
    /*定义一个 GPIO_InitTypeDef 类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启 GPIOD 的外设时钟*/
    RCC_APB2PeriphClockCmd(macSOLENOID1_GPIO_CLK, ENABLE); 

    /*选择要控制的 GPIOD 引脚*/
    GPIO_InitStructure.GPIO_Pin = macSOLENOID1_GPIO_PIN;

    /*设置引脚模式为通用推挽输出*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

    /*设置引脚速率为50MHz*/   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

    /*调用库函数，初始化 GPIOD2*/
    GPIO_Init(macSOLENOID1_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    
    /*选择要控制的 GPIOD 引脚*/
    GPIO_InitStructure.GPIO_Pin = macSOLENOID2_GPIO_PIN;
    
    /*调用库函数，初始化 GPIOD1*/
    GPIO_Init(macSOLENOID2_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    
    /*关闭所有电磁阀*/
    GPIO_ResetBits(macSOLENOID1_GPIO_PORT, macSOLENOID1_GPIO_PIN);
    GPIO_ResetBits(macSOLENOID2_GPIO_PORT, macSOLENOID2_GPIO_PIN);
}
/*********************************************END OF FILE**********************/
