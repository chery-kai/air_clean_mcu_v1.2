/**
  ******************************************************************************
  * @file    bsp_liquid_sensor.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   液位传感器应用函数接口
  ******************************************************************************
  * LIQUID_SENSOR 使用的引脚:
  *     原液箱液位传感器:
  *         LOW_Y: PD8
  *         MID_Y: PD9
  *         HIGH_Y: PD10  
  *     工作仓液位传感器:
  *         LOW_W: PD11
  *         HIGH_W: PD12
  *
  * @工作原理
  *     液位传感器: 读取液位对应引脚的电平, 当为低电平时, 引脚对应的液位生效
  *     例如: 读取到 MID_Y 为低电平, 则此时原液箱为中液位
  * 
  ******************************************************************************
  */
  
#include "bsp_liquid_sensor.h"     

 /**
  * @brief  初始化控制液位传感器的IO
  * @param  无
  * @retval 无
  */
void LIQUID_SENSOR_GPIO_Config(void)
{
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启 GPIOD 的外设时钟*/
    RCC_APB2PeriphClockCmd(macLIQUID_SENSOR_GPIO_CLK, ENABLE); 
    
    //*************** 原液箱液位对应引脚号初始化 ***************
    //手动版
    /*
    GPIO_InitStructure.GPIO_Pin = macSENSOR_LOW_Y_GPIO_PIN | 
            macSENSOR_MID_Y_GPIO_PIN | macSENSOR_HIGH_Y_GPIO_PIN;
    */
    //自动版
    GPIO_InitStructure.GPIO_Pin = macSENSOR_LOW_Y_GPIO_PIN | macSENSOR_HIGH_Y_GPIO_PIN;

    /*设置引脚模式为浮空输入*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   

    /*设置引脚速率为50MHz, 这里应该可以不设置, 设置应该也没有影响*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

    /*调用库函数，初始化 GPIOD8--GPIOD10*/
    GPIO_Init(macLIQUID_SENSOR_GPIO_PORT, &GPIO_InitStructure);

    //*************** 工作仓液位对应引脚号初始化 ***************
    GPIO_InitStructure.GPIO_Pin = macSENSOR_LOW_W_GPIO_PIN | macSENSOR_HIGH_W_GPIO_PIN;

    /*调用库函数，初始化 GPIOD11--GPIOD15*/
    GPIO_Init(macLIQUID_SENSOR_GPIO_PORT, &GPIO_InitStructure);
}


/*
***********************************************************
*	函 数 名: get_liquid_level_sensor_status
*	功能说明: 获取液位传感器的值
*	形    参：无
*	返 回 值: 无
***********************************************************
*/
char get_liquid_level_sensor_status(char sensor)
{
    switch(sensor)
    {
        //原液箱低液位
        case LIQUID_LEVEL_SENSOR_ORG_LOW:
            /*
            if((macSENSOR_LOW_Y_Read()==LOW_LEV) && 
                    (macSENSOR_MID_Y_Read()==HIGH_LEV) && 
                    (macSENSOR_HIGH_Y_Read()==HIGH_LEV))
                return 1;
            else
                return 0;
            */
            if(macSENSOR_LOW_Y_Read()==LOW_LEV)
                return 0;
            else
                return 1;
        
        //原液箱中液位
        case LIQUID_LEVEL_SENSOR_ORG_MID:
            /*
            if((macSENSOR_LOW_Y_Read()==HIGH_LEV) && 
                    (macSENSOR_MID_Y_Read()==LOW_LEV) && 
                    (macSENSOR_HIGH_Y_Read()==HIGH_LEV))
                return 1;
            else
                return 0;
            */
            //手动版
            /*
            if(macSENSOR_MID_Y_Read()==LOW_LEV)
                return 0;
            else
                return 1;
            */
            //自动版
            if((macSENSOR_LOW_Y_Read()==HIGH_LEV) && 
                    (macSENSOR_HIGH_Y_Read()==HIGH_LEV))
                return 0;
            else
                return 1;
        
        //原液箱高液位
        case LIQUID_LEVEL_SENSOR_ORG_HIGH:
            /*
            if((macSENSOR_LOW_Y_Read()==HIGH_LEV) && 
                    (macSENSOR_MID_Y_Read()==HIGH_LEV) && 
                    (macSENSOR_HIGH_Y_Read()==LOW_LEV))
                return 1;
            else
                return 0;
            */
            if(macSENSOR_HIGH_Y_Read()==LOW_LEV)
                return 0;
            else
                return 1;
        
        //工作仓低液位
        case LIQUID_LEVEL_SENSOR_WORK_LOW:
            /*
            if((macSENSOR_LOW_W_Read()==LOW_LEV) && 
                    (macSENSOR_HIGH_W_Read()==HIGH_LEV))
                return 1;
            else
                return 0;
            */
            if(macSENSOR_LOW_W_Read()==LOW_LEV)
                return 0;
            else
                return 1;
        /*
        //工作仓中液位
        case LIQUID_LEVEL_SENSOR_WORK_MID:
            if(macSENSOR_MID_W_Read()==LOW_LEV)
                return 0;
            else
                return 1;
        */
						
        
        //工作仓高液位
        case LIQUID_LEVEL_SENSOR_WORK_HIGH:
            /*
            if((macSENSOR_LOW_W_Read()==HIGH_LEV) && 
                    (macSENSOR_HIGH_W_Read()==LOW_LEV))
                return 1;
            else
                return 0;
            */
            if(macSENSOR_HIGH_W_Read()==LOW_LEV)
                return 0;
            else
                return 1;
        
				//工作仓溢出液位
        case LIQUID_LEVEL_SENSOR_WORK_OVERFLOW:
            if(macSENSOR_OFL_W_Read()==LOW_LEV)
                return 0;
            else
                return 1;
						
        default:
            return -1;
    }
}


/*********************************************END OF FILE**********************/
