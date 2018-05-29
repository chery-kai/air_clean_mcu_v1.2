/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   atomizer 应用函数接口
  ******************************************************************************
  * ATOMIZER 使用的引脚:
  *     WHQ_CTL0:   PD4     -- 雾化器A
  *     WHQ_CTL1:   PD3     -- 雾化器B
  */
  
#include "bsp_atomizer.h"     

 /**
  * @brief  初始化控制 ATOMIZER 的IO
  * @param  无
  * @retval 无
  */
void ATOMIZER_GPIO_Config(void)
{   
    /*定义一个 GPIO_InitTypeDef 类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启 GPIOD 的外设时钟*/
    RCC_APB2PeriphClockCmd(macATOMIZER_A_GPIO_CLK, ENABLE); 

    /*选择要控制的 GPIOD 引脚*/
    GPIO_InitStructure.GPIO_Pin = macATOMIZER_A_GPIO_PIN;

    /*设置引脚模式为通用推挽输出*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

    /*设置引脚速率为50MHz*/   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

    /*调用库函数，初始化 GPIOD4*/
    GPIO_Init(macATOMIZER_A_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    
    /*选择要控制的 GPIOD 引脚*/
    GPIO_InitStructure.GPIO_Pin = macATOMIZER_B_GPIO_PIN;
    
    /*调用库函数，初始化 GPIOD3*/
    GPIO_Init(macATOMIZER_B_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    
    /*关闭所有雾化器*/
    GPIO_ResetBits(macATOMIZER_A_GPIO_PORT, macATOMIZER_A_GPIO_PIN);
    GPIO_ResetBits(macATOMIZER_B_GPIO_PORT, macATOMIZER_B_GPIO_PIN);
}
/*********************************************END OF FILE**********************/